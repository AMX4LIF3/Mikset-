#include "common.hpp"

std::string ReadFileToString(const std::string& location) {
	std::ifstream file(location);
	if (!file.is_open()) {
		std::string err = "Failed to open file '" + location + "'";
		MessageBoxA(NULL, err.c_str(), "ReadFileToString Failed", MB_OK | MB_ICONERROR);
		exit(-1);
	}

	std::string content(std::istreambuf_iterator<char>(file), {});
	return content;
}

XMFLOAT3 XMFLOAT3MUL(XMFLOAT3 A, XMFLOAT3 B) {
	XMVECTOR vA = XMLoadFloat3(&A);
	XMVECTOR vB = XMLoadFloat3(&B);
	XMVECTOR vResult = vA * vB;

	XMFLOAT3 result;
	XMStoreFloat3(&result, vResult);
	return result;
}




