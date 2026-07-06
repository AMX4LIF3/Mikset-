#include "scene.hpp"
#include <iostream>

Scene::Scene() {
	gb.LightDir = XMFLOAT4(10.0f, -100.0f, 10.0f, 0.0f);

}

Scene::~Scene() {

}

static D3D11_BUFFER_DESC MakeCBDesc(UINT byteWidth) {
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = ((byteWidth + 15) / 16) * 16; // 16-byte align
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	return desc;
}

bool set = false;
bool Scene::Begin(Core& core, Camera& cam, Shader& shader) {
	this->camera = &cam;
	this->viewProjCache = this->camera->GetViewProjection();
	auto* ctx = core.m_deviceCtx.Get();

	// force Main Shaders back onto the pipeline
	ctx->VSSetShader(shader.GetVS().Get(), nullptr, 0);
	ctx->PSSetShader(shader.GetPS().Get(), nullptr, 0);
	ctx->PSSetSamplers(0, 1, core.m_sampler.GetAddressOf());


	// local array of pointers not GetAddressOf() on copies
	ID3D11ShaderResourceView* nullSRVs[5] = { nullptr };
	ctx->PSSetShaderResources(0, 5, nullSRVs);

	if (this->SkyBox) {
		ID3D11ShaderResourceView* srv = SkyBox->GetSRV().Get();
		ctx->PSSetShaderResources(10, 1, &srv);
	}

	// bind shaders
	core.m_deviceCtx->VSSetShader(shader.GetVS().Get(), nullptr, 0);
	core.m_deviceCtx->PSSetShader(shader.GetPS().Get(), nullptr, 0);
	core.m_deviceCtx->PSSetSamplers(0, 1, core.m_sampler.GetAddressOf());

	// create buffesr
	HRESULT hr;
	if (!globBuff) {
		D3D11_BUFFER_DESC gdesc = MakeCBDesc(sizeof(GlobalBuffer));
		hr = core.m_device->CreateBuffer(&gdesc, nullptr, globBuff.GetAddressOf());
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to create global buffer..", "SceneBegin(): CreateBuffer() failed..", MB_OK | MB_ICONERROR);
			return false;
		}
	}
	if (!objBuff) {
		D3D11_BUFFER_DESC odesc = MakeCBDesc(sizeof(ObjectBuffer));
		hr = core.m_device->CreateBuffer(&odesc, nullptr, objBuff.GetAddressOf());
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to create object buffer..", "SceneBegin(): CreateBuffer() failed..", MB_OK | MB_ICONERROR);
			return false;
		}
	}
	if (!matBuff) {
		D3D11_BUFFER_DESC mdesc = MakeCBDesc(sizeof(MaterialBuffer));
		hr = core.m_device->CreateBuffer(&mdesc, nullptr, matBuff.GetAddressOf());
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to create material buffer..", "SceneBegin(): CreateBuffer() failed..", MB_OK | MB_ICONERROR);
			return false;
		}
	}
	if (!skinningBuff) {
		D3D11_BUFFER_DESC skindesc = MakeCBDesc(sizeof(SkinningBuffer));
		hr = core.m_device->CreateBuffer(&skindesc, nullptr, skinningBuff.GetAddressOf());
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to create skinning buffer..", "SceneBegin(): CreateBuffer() failed..", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	gb.viewProj = XMMatrixTranspose(this->viewProjCache);
	gb.CameraPos = XMFLOAT4(this->camera->pos.x, this->camera->pos.y, this->camera->pos.z, 1.0f);

	// normalize lightdir before upload to the shader so shader normalizes too and behavior is stable.
	{
		XMVECTOR ld = XMLoadFloat4(&gb.LightDir);
		ld = XMVector3Normalize(ld);
		XMStoreFloat4(&gb.LightDir, ld);
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	hr = core.m_deviceCtx->Map(globBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to map a global buffer.. ", "Scene::Begin(): Map() failed..", MB_OK | MB_ICONERROR);
		return false;
	}


	memcpy(mappedResource.pData, &gb, sizeof(GlobalBuffer));
	core.m_deviceCtx->Unmap(globBuff.Get(), 0);

	core.m_deviceCtx->VSSetConstantBuffers(0, 1, globBuff.GetAddressOf());
	core.m_deviceCtx->PSSetConstantBuffers(0, 1, globBuff.GetAddressOf());

	core.m_deviceCtx->VSSetConstantBuffers(1, 1, objBuff.GetAddressOf());
	core.m_deviceCtx->PSSetConstantBuffers(1, 1, objBuff.GetAddressOf());

	core.m_deviceCtx->VSSetConstantBuffers(2, 1, matBuff.GetAddressOf());
	core.m_deviceCtx->PSSetConstantBuffers(2, 1, matBuff.GetAddressOf());

	core.m_deviceCtx->VSSetConstantBuffers(3, 1, skinningBuff.GetAddressOf());
	core.m_deviceCtx->PSSetConstantBuffers(3, 1, skinningBuff.GetAddressOf());

	// reset tracking each frame
	lastSkinnedModelUploaded = nullptr;

	return true;
}


void Scene::Submit(Core& core, Entity* entity, XMMATRIX transform) {
	if (entity->MulByRootNodeTransf) entity->worldMat = transform * entity->GetModel()->GetRootNodeTransformXM();
	else entity->worldMat = transform;
	renderQueue.push_back(entity);
}

void Scene::SubmitWithCustomView(Core& core, Entity* entity, XMMATRIX transform, XMMATRIX view) { // will prob have with custom proj aswell
	// but for now couldnt care less
	ObjectBuffer oj;
	oj.World = XMMatrixTranspose(transform);
	oj.WVP = XMMatrixTranspose(transform * view * this->camera->GetProjMatrix());
	oj.HasBones = entity->GetModel()->HasBones ? 1 : 0;

	D3D11_MAPPED_SUBRESOURCE objMapped{};
	HRESULT hr = core.m_deviceCtx->Map(objBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &objMapped);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to map a object buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
		return;
	}
	else {
		memcpy(objMapped.pData, &oj, sizeof(ObjectBuffer));
		core.m_deviceCtx->Unmap(objBuff.Get(), 0);
	}

	D3D11_MAPPED_SUBRESOURCE matMapped{};
	// DEBUG: print material before upload
	//std::cout << "Submitting entity material metallic (CPU) = " << entity->material.Metallic << std::endl;
	hr = core.m_deviceCtx->Map(matBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &matMapped);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to map a Material buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
		return;
	}
	else {
		memcpy(matMapped.pData, &entity->material, sizeof(MaterialBuffer));
		// DEBUG: inspect what we're uploading
		MaterialBuffer* uploaded = reinterpret_cast<MaterialBuffer*>(matMapped.pData);
		//std::cout << "Uploading metallic (mapped memory) = " << uploaded->Metallic << std::endl;
		core.m_deviceCtx->Unmap(matBuff.Get(), 0);
	}
	core.m_deviceCtx->PSSetConstantBuffers(2, 1, matBuff.GetAddressOf());

	if (entity->GetModel()->HasBones) {
		Model* modelPtr = entity->GetModel();
		if (modelPtr != lastSkinnedModelUploaded) {
			D3D11_MAPPED_SUBRESOURCE skinningMapped{};
			hr = core.m_deviceCtx->Map(skinningBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &skinningMapped);
			if (FAILED(hr)) {
				MessageBoxA(NULL, "failed to map a Skinning buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
				// fallthrough: don't return, just skip skinning upload for this model
			}
			else {
				memcpy(skinningMapped.pData, modelPtr->GetBonePallete(), sizeof(XMMATRIX) * MAX_BONE_COUNT);
				core.m_deviceCtx->Unmap(skinningBuff.Get(), 0);
				lastSkinnedModelUploaded = modelPtr;
			}
		}

		core.m_deviceCtx->VSSetConstantBuffers(3, 1, skinningBuff.GetAddressOf());
	}

	entity->GetModel()->Draw();
}

Entity* Scene::AddEntity(Core& core, const std::string& name, const std::string& path, bool IsCollidable, bool BakeModelTransformations) {
	if (this->modelCache.find(path) == this->modelCache.end()) {
		// if not loaded put it in the shader ptr
		modelCache[path] = std::make_shared<Model>(core, path, !IsCollidable, BakeModelTransformations);
	}

	//auto tar = std::find(this->entities.begin(), this->entities.end(), name);
	for (const auto& entity : this->GetEntities()) {
		if (entity.get()->Name == name) {
			MessageBoxA(NULL, "Entity with matching name already exists..", "AddEntity() Failed!", MB_OK | MB_ICONERROR);
			return nullptr;
		}
	}
	
	auto entity = std::make_unique<Entity>(name, modelCache[path], !IsCollidable);
	if (!entity.get()->GetModel()->IsValid()) {
		modelCache.erase(path);
		return nullptr;
	}

	Entity* entity_ptr = entity.get();


	this->entities.push_back(std::move(entity));
	return entity_ptr;
}

void Scene::RemoveEntity(Entity* target) {
	if (!target) {
		MessageBoxA(NULL, "Failed to remove entity...", "RemoveEntity() Failed!", MB_OK | MB_ICONERROR);
		return;
	}

	if (lastSkinnedModelUploaded == target->GetModel()) lastSkinnedModelUploaded = nullptr; 

	//this->modelCache.erase(target->GetModel()->GetPath());

	// erase the entity data
	std::erase_if(this->entities, [target](const std::unique_ptr<Entity>& e) { return e.get() == target; });
/*
	const auto it = this->modelCache.find(path);
	if (it != this->modelCache.end()) {
		// no other entities are using it so fuck off
		this->modelCache.erase(it);
	}
*/
}

// TODO: calling RemoveEntity on an entity that wasnt the last one uploaded doesnt release the memory.
void Scene::RemoveEntityByName(std::string target) {

	std::string cpy = lastSkinnedModelUploaded->GetPath();
	if (lastSkinnedModelUploaded->GetPath() == target) lastSkinnedModelUploaded = nullptr;
		
	this->modelCache.erase(cpy);

	auto removedCount = std::erase_if(this->entities, [&target](const std::unique_ptr<Entity>& e) {
		return e->Name == target;
	});
	if (removedCount == 0) {
		MessageBoxA(NULL, "No Entity exists with the specified Name..\nPlease check spelling.", "RemoveEntityByName() Failed!", MB_OK | MB_ICONERROR);
		return;
	}

	const auto it = this->modelCache.find(cpy);
	if (it != this->modelCache.end()) {
		this->modelCache.erase(it);
	}

}


std::vector<std::unique_ptr<Entity>>& Scene::GetEntities() { return this->entities; }

std::shared_ptr<Model> Scene::GetModel(Core& core, const std::string& path, bool IsCollidable) {
	if (this->modelCache.find(path) == this->modelCache.end()) {
		this->modelCache[path] = std::make_shared<Model>(core, path, IsCollidable);
	}
	return this->modelCache[path];
}

ComPtr<ID3D11Buffer>& Scene::GetGlobalBuffer() { return this->globBuff; }
ComPtr<ID3D11Buffer>& Scene::GetObjectBuffer() { return this->objBuff; }
ComPtr<ID3D11Buffer>& Scene::GetMaterialBuffer() { return this->matBuff; }

bool Scene::SetUpBuffers(Core& core) {
	shadowmap.Init(core);
	// setting up a constant buffer
	D3D11_BUFFER_DESC desc{};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	desc.ByteWidth = sizeof(GlobalBuffer);
	HRESULT hr = core.m_device->CreateBuffer(&desc, nullptr, this->globBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to create a global buffer..", "Failed to CreateBuffer", MB_OK | MB_ICONERROR);
		return false;
	}
	desc.ByteWidth = sizeof(ObjectBuffer);
	hr = core.m_device->CreateBuffer(&desc, nullptr, this->objBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to create an object buffer..", "Failed to CreateBuffer", MB_OK | MB_ICONERROR);
		return false;
	}

	desc.ByteWidth = sizeof(MaterialBuffer);
	hr = core.m_device->CreateBuffer(&desc, nullptr, this->matBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to create a material buffer..", "Failed to CreateBuffer", MB_OK | MB_ICONERROR);
		return false;
	}

	// create skinning buffer as well so we can upload skeletons
	desc.ByteWidth = sizeof(SkinningBuffer);
	hr = core.m_device->CreateBuffer(&desc, nullptr, this->skinningBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "failed to create a skinning buffer..", "Failed to CreateBuffer", MB_OK | MB_ICONERROR);
		return false;
	}
/*
	D3D11_INPUT_ELEMENT_DESC shadowLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
*/
	// set up shadow shader
	shadowShader.VSCompileFromFile(core, core.GetDevice(), "Renderer/Graphics/shaders/shadow.hlsl", "Shadow_Shader");

	core.m_device->CreateInputLayout(MainShaderIE_desc, _countof(MainShaderIE_desc),
		shadowShader.GetBlob()->GetBufferPointer(), shadowShader.GetBlob()->GetBufferSize(),
		shadowShader.GetIL().GetAddressOf());


	return true;
}

Camera lightcam;

void Scene::UpdateShadowMap(Core& core, Player* player) {
	if (!player) return;

	XMVECTOR sceneCenter = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR vLightDir = XMVector3Normalize(XMLoadFloat4(&gb.LightDir));

	XMVECTOR eyepos = sceneCenter - (vLightDir * 150);

	float dotUp = fabs(XMVectorGetY(vLightDir));
	XMVECTOR up = (dotUp > 0.99f) ? XMVectorSet(0, 0, 1, 0) : XMVectorSet(0, 1, 0, 0);
	XMMATRIX lView = XMMatrixLookToLH(eyepos, vLightDir, up);

	float sceneSize = 300.0f;
	XMMATRIX lProj = XMMatrixOrthographicLH(sceneSize, sceneSize, 0.1f, 500.0f);

	XMMATRIX vp = lView * lProj;
	XMStoreFloat4x4(&this->lightViewProj, vp);

	XMStoreFloat3(&lightcam.pos, eyepos);
}


void SaveShadowMapToPNG(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* shadowTexture, const wchar_t* filename) {
	D3D11_TEXTURE2D_DESC desc;
	shadowTexture->GetDesc(&desc);

	// 1. Create a staging texture to copy the depth data into (CPU readable)
	D3D11_TEXTURE2D_DESC stagingDesc = desc;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.BindFlags = 0; // Staging textures cannot be bound to the pipeline
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
	HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, stagingTexture.GetAddressOf());
	if (FAILED(hr)) return;

	// 2. Copy the GPU-only shadow map into our staging texture
	context->CopyResource(stagingTexture.Get(), shadowTexture);

	// 3. Map the memory so we can extract the depth floats
	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hr)) return;

	// Since our format is R32_TYPELESS / D32_FLOAT, each pixel is a 32-bit float
	float* depthData = reinterpret_cast<float*>(mapped.pData);
	UINT width = desc.Width;
	UINT height = desc.Height;
	UINT stride = mapped.RowPitch / sizeof(float);

	// 4. Convert the 32-bit depth float array into regular 8-bit grayscale pixels (0-255)
	// This makes it a viewable image format.
	std::vector<uint8_t> rgbaData(width * height * 4);
	for (UINT y = 0; y < height; ++y) {
		for (UINT x = 0; x < width; ++x) {
			float z = depthData[y * stride + x];

			// Convert depth [0.0, 1.0] to grayscale byte [0, 255]
			uint8_t colorValue = static_cast<uint8_t>(z * 255.0f);

			UINT pixelOffset = (y * width + x) * 4;
			rgbaData[pixelOffset + 0] = colorValue; // R
			rgbaData[pixelOffset + 1] = colorValue; // G
			rgbaData[pixelOffset + 2] = colorValue; // B
			rgbaData[pixelOffset + 3] = 255;        // A
		}
	}
	context->Unmap(stagingTexture.Get(), 0);

	// 5. Use DirectXTex to write the data out to disk
	DirectX::Image img;
	img.width = width;
	img.height = height;
	img.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	img.rowPitch = width * 4;
	img.slicePitch = img.rowPitch * height;
	img.pixels = rgbaData.data();

	hr = DirectX::SaveToWICFile(img, DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), filename);
	if (SUCCEEDED(hr)) {
		std::cout << "Shadow map successfully saved to disk!" << std::endl;
	}
}


void Scene::End(Core& core, Shader shader) {

	// 2 passes
	// 1 for shadows and 1 for entities..rough but dont complain too much

	shadowmap.BindAsTarget(core);
	//core.m_deviceCtx->ClearDepthStencilView(shadowmap.dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	GlobalBuffer globbuf{};
	globbuf.viewProj = XMMatrixTranspose(XMLoadFloat4x4(&lightViewProj)); // use light VP as the main VP for shadows and in theory this SHOULD work..
	globbuf.lightViewProj = globbuf.viewProj;
	globbuf.CameraPos = XMFLOAT4(lightcam.pos.x, lightcam.pos.y, lightcam.pos.z, 1.0f);
	//if (!set) { 
		//globbuf.LightDir = gb.LightDir;
		//set = true;
	//}
	globbuf.LightDir = gb.LightDir;
	D3D11_MAPPED_SUBRESOURCE mapped;
	core.m_deviceCtx->Map(globBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &globbuf, sizeof(GlobalBuffer));
	core.m_deviceCtx->Unmap(globBuff.Get(), 0);
	core.m_deviceCtx->VSSetConstantBuffers(0, 1, globBuff.GetAddressOf());
	core.m_deviceCtx->PSSetConstantBuffers(0, 1, globBuff.GetAddressOf());

	core.m_deviceCtx->RSSetViewports(1, &shadowmap.viewport);
	core.m_deviceCtx->VSSetShader(shadowShader.GetVS().Get(), nullptr, 0);

	// first pass
	for (const auto& entity : renderQueue) {
		oj.World = XMMatrixTranspose(entity->worldMat);
		oj.WVP = XMMatrixTranspose(entity->worldMat * XMLoadFloat4x4(&lightViewProj));

		oj.HasBones = entity->GetModel()->HasBones ? 1 : 0;

		D3D11_MAPPED_SUBRESOURCE objMapped{};
		HRESULT hr = core.m_deviceCtx->Map(objBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &objMapped);
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to map a object buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
			return;
		}
		else {
			memcpy(objMapped.pData, &oj, sizeof(ObjectBuffer));
			core.m_deviceCtx->Unmap(objBuff.Get(), 0);
		}
		core.m_deviceCtx->VSSetConstantBuffers(1, 1, objBuff.GetAddressOf());
		core.m_deviceCtx->PSSetConstantBuffers(1, 1, objBuff.GetAddressOf());



		if (entity->GetModel()->HasBones)
		{
			Model* modelPtr = entity->GetModel();

			D3D11_MAPPED_SUBRESOURCE skinningMapped{};
			hr = core.m_deviceCtx->Map(skinningBuff.Get(), 0,
				D3D11_MAP_WRITE_DISCARD, 0, &skinningMapped);

			if (SUCCEEDED(hr))
			{
				memcpy(skinningMapped.pData,
					modelPtr->GetBonePallete(),
					sizeof(XMMATRIX) * MAX_BONE_COUNT);

				core.m_deviceCtx->Unmap(skinningBuff.Get(), 0);
			}

			core.m_deviceCtx->VSSetConstantBuffers(3, 1,
				skinningBuff.GetAddressOf());
		}
		entity->GetModel()->Draw();
	}


	// second pass 
	ID3D11RenderTargetView* nullRTV = nullptr;
	core.m_deviceCtx->OMSetRenderTargets(1, &nullRTV, nullptr);

	shadowmap.BindAsResource(core);

	globbuf.viewProj = XMMatrixTranspose(this->viewProjCache);
	globbuf.lightViewProj = XMMatrixTranspose(XMLoadFloat4x4(&lightViewProj));
	globbuf.CameraPos = XMFLOAT4(this->camera->pos.x, this->camera->pos.y, this->camera->pos.z, 1.0f);
	globbuf.LightDir = gb.LightDir;

	//	if (!set) { 
		//	globbuf.LightDir = gb.LightDir;
			//set = true;
		//}

	core.m_deviceCtx->IASetInputLayout(shader.m_IL.Get());
	D3D11_MAPPED_SUBRESOURCE mapped2;
	core.m_deviceCtx->Map(globBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped2);
	memcpy(mapped2.pData, &globbuf, sizeof(GlobalBuffer));
	core.m_deviceCtx->Unmap(globBuff.Get(), 0);

	core.m_deviceCtx->OMSetRenderTargets(1, core.m_rtv.GetAddressOf(), core.m_dsv.Get());
	core.ClearDepth();
	core.m_deviceCtx->RSSetViewports(1, &core.m_viewPort);
	core.m_deviceCtx->VSSetShader(shader.GetVS().Get(), nullptr, 0);
	core.m_deviceCtx->PSSetShader(shader.GetPS().Get(), nullptr, 0);

	core.m_deviceCtx->PSSetSamplers(0, 1, core.m_sampler.GetAddressOf());
	for (const auto& entity : renderQueue) {
		oj.World = XMMatrixTranspose(entity->worldMat);
		oj.WVP = XMMatrixTranspose(entity->worldMat * this->viewProjCache);


		oj.HasBones = entity->GetModel()->HasBones ? 1 : 0;

		D3D11_MAPPED_SUBRESOURCE objMapped{};
		HRESULT hr = core.m_deviceCtx->Map(objBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &objMapped);
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to map a object buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
			return;
		}
		else {
			memcpy(objMapped.pData, &oj, sizeof(ObjectBuffer));
			core.m_deviceCtx->Unmap(objBuff.Get(), 0);
		}

		D3D11_MAPPED_SUBRESOURCE matMapped{};
		// DEBUG: print material before upload
		//std::cout << "Submitting entity material metallic (CPU) = " << entity->material.Metallic << std::endl;
		hr = core.m_deviceCtx->Map(matBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &matMapped);
		if (FAILED(hr)) {
			MessageBoxA(NULL, "failed to map a Material buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
			return;
		}
		else {
			memcpy(matMapped.pData, &entity->material, sizeof(MaterialBuffer));
			// DEBUG: inspect what we're uploading
			MaterialBuffer* uploaded = reinterpret_cast<MaterialBuffer*>(matMapped.pData);
			//std::cout << "Uploading metallic (mapped memory) = " << uploaded->Metallic << std::endl;
			core.m_deviceCtx->Unmap(matBuff.Get(), 0);
		}
		core.m_deviceCtx->PSSetConstantBuffers(2, 1, matBuff.GetAddressOf());

		// upload skinning data only when model has bones and only once per model per frame
		if (entity->GetModel()->HasBones) {
			Model* modelPtr = entity->GetModel();
			if (modelPtr != lastSkinnedModelUploaded) {
				D3D11_MAPPED_SUBRESOURCE skinningMapped{};
				hr = core.m_deviceCtx->Map(skinningBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &skinningMapped);
				if (FAILED(hr)) {
					MessageBoxA(NULL, "failed to map a Skinning buffer.. ", "Scene::Submit(): Map() failed..", MB_OK | MB_ICONERROR);
					// this would skip skinning for the model as a fallthrough mechanism kinda
				}
				else {
					memcpy(skinningMapped.pData, modelPtr->GetBonePallete(), sizeof(XMMATRIX) * MAX_BONE_COUNT);
					core.m_deviceCtx->Unmap(skinningBuff.Get(), 0);
					lastSkinnedModelUploaded = modelPtr;
				}
			}

			core.m_deviceCtx->VSSetConstantBuffers(3, 1, skinningBuff.GetAddressOf());
		}

		entity->GetModel()->Draw();
	}
	//SaveShadowMapToPNG(core.m_device.Get(), core.m_deviceCtx.Get(), shadowmap.tex.Get(), L"shadow_debug.png");
	renderQueue.clear(); // reset
}




