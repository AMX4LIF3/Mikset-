// would probably be faster and maybe more effecient (depending on who you ask) 
// if i use enums to avoid string comparisions. but im on a modern system here so it shouldn't be much 
// an issue


#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>


#define Entity "0x67\n"
#define EntityTerm "0x69\n"

enum CollisionType { Box, TriangleMesh };
struct Collider { CollisionType type; float Min, Max; };
struct float3 { float x, y, z; };
struct Transform { float3 Translation, Rotation, Scale; };
struct Material { float roughness, metallic, ao; };

class entity {
    public:
        std::string name;
        Transform transf;
        Material material;
        std::vector<Collider> colliders;
        std::string path;
};

class Scene {
    public: 
        std::vector<entity> entities;
};

void LogEntity(entity entity, std::string filename) {
    std::ofstream target(filename, std::ios::app);

    if (target.is_open()) {
        int i = 0;
        target << Entity; // special character to define a new entity

        target << "ID : " << entity.name << '\n';
        target << "Path : " << entity.path << '\n';

        // Transformation 
        target << "TranslationX : " << entity.transf.Translation.x << '\n';
        target << "TranslationY : " << entity.transf.Translation.y << '\n';
        target << "TranslationZ : " << entity.transf.Translation.z << '\n';

        target << "RotationX : " << entity.transf.Rotation.x << '\n';
        target << "RotationY : " << entity.transf.Rotation.y << '\n';
        target << "RotationZ : " << entity.transf.Rotation.z << '\n';

        target << "ScaleX : " << entity.transf.Scale.x << '\n';
        target << "ScaleY : " << entity.transf.Scale.y << '\n';
        target << "ScaleZ : " << entity.transf.Scale.z << '\n';


        // Materials
        target << "Roughness : " << entity.material.roughness << '\n';
        target << "Metallic : " << entity.material.metallic << '\n';
        target << "AO : " << entity.material.ao << '\n';

        // Colliders
        for (const auto& col : entity.colliders) {
            i++;
            if (col.type == CollisionType::TriangleMesh) {
                target << "Push : TriangleMesh\n";
            }else if (col.type == CollisionType::Box) { // now its getting intense
                target << "Collider#" << i << " : " << "Min : " << col.Min << " Max : " << col.Max << "\n" ;
            }
        }
        

        target << EntityTerm; // you are done with the entity
    }


    target.close();
}

void tokenize(std::string key, std::string value, entity& dummy) {
    if (key.empty() || value.empty()) {
        std::cout << "WARNING EMPTY KEY OR VALUE\n";
        return;
    }

    if (key == "ID ") dummy.name = value;                
    else if (key == "Path ") dummy.path = value;
    else if (key == "TranslationX ") dummy.transf.Translation.x = std::stof(value);
    else if (key == "TranslationY ") dummy.transf.Translation.y = std::stof(value);
    else if (key == "TranslationZ ") dummy.transf.Translation.z = std::stof(value);

    else if (key == "RotationX ")   dummy.transf.Rotation.x = std::stof(value);
    else if (key == "RotationY ")   dummy.transf.Rotation.y = std::stof(value);
    else if (key == "RotationZ ")   dummy.transf.Rotation.z = std::stof(value);

    else if (key == "ScaleX ")      dummy.transf.Scale.x = std::stof(value);
    else if (key == "ScaleY ")      dummy.transf.Scale.y = std::stof(value);
    else if (key == "ScaleZ ")      dummy.transf.Scale.z = std::stof(value);

    else if (key == "Roughness ") dummy.material.roughness = std::stof(value);
    else if (key == "Metallic ") dummy.material.metallic = std::stof(value);
    else if (key == "AO ") dummy.material.ao = std::stof(value);

    else if (key == "Push ") {
        if (value == " TriangleMesh" || value == "TriangleMesh") {
            Collider col; 
            col.type = CollisionType::TriangleMesh;
            dummy.colliders.push_back(col);
        }else if(value == " Box" || value == "Box") {
            /// TODO
        }else {
            std::cout << "couldnt assign collder..\n";
        }
    }else {
        std::cout << "No KeyWord Defined As [" + key + "] found..\n";
    }

}

//entity target;
std::vector<entity> LoadLog(std::string filename, std::vector<entity>& target) {
    std::ifstream source(filename);

    //std::string assignOP = ":";

    std::string currentKey = "";
    std::string currentValue = "";
    bool buildingKey = true;
    bool buildingValue = false;
    entity dummy;
  
    bool inEntity;
    std::string line;
    while (std::getline(source, line)) {

        // strip these annoying shits because fuckiing windows wooo
        if (line.empty() && line.back() == '/r') line.pop_back();

        if (line == "0x67") {
            dummy = entity(); // clear it 
            inEntity = true;
            continue; 
        }   

        if (line == "0x69") {
            if (inEntity) {
                target.push_back(dummy); // save it 
                inEntity = false;
            }

            continue;
        }

        size_t colonPos = line.find(":"); // because : acts as the assign operator we have to detect it 
        if (colonPos == std::string::npos) continue; // not valid -> skip
        
        // now with operator pos we know exactly where the colon is
        // finding the colon means that theres a key to the left and its value is right 
        // relative to the colon, we know the starting and ending position using the \n symbol
        currentKey = line.substr(0, colonPos); 
        currentValue = line.substr(colonPos + 1);
        //std::cout << currentKey << " : " << currentValue << '\n';

        // clean it up from empty spaces
        if (!currentValue.empty() && currentValue.front() == ' ') currentValue.erase(0, 1);
        tokenize(currentKey, currentValue, dummy);
        
    }

  //  target.push_back(dummy);
    return target;
}
auto main()-> int{
    Scene scene;
/*
    entity platform;
    platform.name = "hey";
    platform.transf.Translation = {58.0f, 60.0f, 100.0f};
    platform.transf.Rotation = {1.57f, 0.0f, 0.0f};
    platform.transf.Scale = {1.0f, 1.0f, 1.0f};
    platform.material.roughness = 0.5f;
    platform.material.metallic = 0.0f;
    platform.material.ao = 1.0f;

    Collider platformCol; 
    platformCol.type = CollisionType::TriangleMesh;
    platform.colliders.push_back(platformCol);
    scene.entities.push_back(platform);

    entity wall;
    wall.name = "wall";
    wall.transf.Translation = {518.0f, 60.0f, 100.0f};
    wall.transf.Rotation = {1.57f, 1.0f, 0.0f};
    wall.transf.Scale = {1.0f, 1.0f, 1.0f};
    wall.material.roughness = 0.5f;
    wall.material.metallic = 1.0f;
    wall.material.ao = 1.0f;

    Collider wallCol; 
    wallCol.type = CollisionType::TriangleMesh;
    wall.colliders.push_back(wallCol);
    scene.entities.push_back(wall);
*/
    
    //for (const auto& entity : scene.entities) LogEntity(entity, "target.mikscene");
    LoadLog("target.mikscene", scene.entities);
    if (!scene.entities.empty()) {
        for (const auto& entity : scene.entities) {
            std::cout << entity.name << '\n'; // prints out empty shit..
        }
    }else {
        std::cout << "scene is empty..\n";
    }
    std::cin.ignore();
    return 0;
}
