#include "HeaderTool.h"
#include <fstream>
#include <filesystem>

void HeaderTool::generate() {
	const std::string headerPath = std::string("../../../Engine/Include/Serializer.h");
	const std::string sourcePath = std::string("../../../Engine/Source/Serializer.cpp");
	LOG_I("Generating...", std::experimental::filesystem::canonical(std::experimental::filesystem::absolute(headerPath)).string());
	const std::string* paths = new std::string[1]{
		"../../../Engine/Include",
	};

	std::stringstream headerEnum;
	headerEnum << generateHeaderEnumBeggining();

	std::stringstream sourceFileFactoryTemplates;
	std::stringstream sourceFileBasicComponents;
	std::stringstream sourceFileIncludes;
	std::stringstream headerFileFactories;
	std::stringstream sourceFileFactories;

	std::string absolutePath = std::experimental::filesystem::canonical(std::experimental::filesystem::absolute(paths[0])).string();
	for (auto& p : std::experimental::filesystem::recursive_directory_iterator(paths[0])) {
		if (std::experimental::filesystem::is_directory(p.path()))
			continue;
		if (p.path().extension() != ".h")
			continue;

		std::string absoluteFile = std::experimental::filesystem::canonical(std::experimental::filesystem::absolute(p.path())).string();
		const std::string relativePath = absoluteFile.substr(absolutePath.size() + 1);
		std::ifstream in(p.path(), std::ios_base::in);
		std::string currentLine;
		while (getline(in, currentLine)) {
			if (currentLine.substr(0, 12) == std::string("FLAVO_SYSTEM")) {
				std::string actualLine = currentLine.substr(13, currentLine.size() - 14);
				const size_t commaPlace = actualLine.find(',');
				const std::string nmspace = actualLine.substr(0, commaPlace);
				std::string className = actualLine.substr(commaPlace + 1, actualLine.size() - commaPlace - 1);
				className.erase(remove_if(className.begin(), className.end(), ::isspace), className.end());

				headerFileFactories << generateTemplateFactoryString(nmspace, className);
				sourceFileIncludes << generateSourceInclude(relativePath);
				sourceFileFactories << generateMapAssignment(nmspace, className);
				sourceFileFactoryTemplates << generateTemplateFactoryDefinition(nmspace, className);
				break;
			} else if (currentLine.substr(0, 15) == std::string("FLAVO_COMPONENT")) {
				std::string actualLine = currentLine.substr(16, currentLine.size() - 17);
				const size_t commaPlace = actualLine.find(',');
				const std::string nmspace = actualLine.substr(0, commaPlace);
				std::string className = actualLine.substr(commaPlace + 1, actualLine.size() - commaPlace - 1);
				className.erase(remove_if(className.begin(), className.end(), ::isspace), className.end());

				static size_t typeIndex = 0;

				headerFileFactories << generateComponentFactoryString(nmspace, className);
				sourceFileIncludes << generateSourceInclude(relativePath);
				sourceFileFactories << generateComponentMapAssignment(nmspace, className);
				sourceFileFactories << generateComponentTypeAssignment(nmspace, className, typeIndex);
				sourceFileFactoryTemplates << generateComponentFactoryDefinition(nmspace, className);
				sourceFileBasicComponents << generateSourceBasicComponent(nmspace, className);
				headerEnum << generateHeaderEnumComponent(className, std::to_string(typeIndex));

				++typeIndex;
				break;
			}
		}

		in.close();
	}

	headerEnum << generateHeaderEnumEnding();

	std::ofstream serializerFile;
	serializerFile.open(headerPath, std::ios_base::out);
	serializerFile << generateHeaderBeggining();
	serializerFile << headerFileFactories.str();
	serializerFile << generateHeaderEnding();
	serializerFile << headerEnum.str();
	serializerFile.close();

	std::ofstream serializerFileSource;
	serializerFileSource.open(sourcePath, std::ios_base::out);
	serializerFileSource << sourceFileIncludes.str();
	serializerFileSource << generateSourceBeggining();
	serializerFileSource << sourceFileFactories.str();
	serializerFileSource << generateSourceEnding();
	serializerFileSource << sourceFileBasicComponents.str();
	serializerFileSource << generateSourceEnding2();
	serializerFileSource << sourceFileFactoryTemplates.str();
	serializerFileSource.close();
}

std::string HeaderTool::generateSourceInclude(const std::string& path) {
	return std::string() + "#include \"" + path + "\"\n";
}

std::string HeaderTool::generateTemplateFactoryString(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"namespace " + nmspace + " { class " + className + "; }\n" +
		"template<> eecs::SystemBase* createObjectOfType<" + typeName + ">();\n\n";
	return retString;
}

std::string HeaderTool::generateComponentFactoryString(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"namespace " + nmspace + " { class " + className + "; }\n" +
		"template<> eecs::ComponentBase* createComponentOfType<" + typeName + ">();\n\n";
	return retString;
}

std::string HeaderTool::generateHeaderBeggining() {
	std::string retString = std::string() +
		"//File generated by Flavo Header Tool. DO NOT edit unless you are sure you know what to do\n" +
		"#pragma once\n\n" +
		"#include \"Global.h\"\n" +
		"#include \"EECS.h\"\n" +
		"typedef eecs::SystemBase*(*CreateObjectOfTypeFunc)();\n" +
		"typedef eecs::ComponentBase*(*CreateComponentOfTypeFunc)();\n\n" +
		"template<typename T> //primary system template\n" +
		"eecs::SystemBase* createObjectOfType() { return new T(); }\n\n" +
		"template<typename T> //primary component template\n" +
		"eecs::ComponentBase* createComponentOfType() { return new T(); }\n\n";
	return retString;
}

std::string HeaderTool::generateHeaderEnding() {
	std::string retString = std::string() +
		"namespace reflection\n" +
		"{\n"
		"	class Serializer\n" +
		"	{\n" +
		"	public:\n" +
		"		Serializer();\n" +
		"		~Serializer();\n\n" +
		"		void initRootComponent(eecs::Entity entity) const;\n\n" +
		"		const std::unordered_map<std::string, CreateObjectOfTypeFunc>& getSystemCreationMap() const;\n" +
		"		const std::unordered_map<std::string, CreateComponentOfTypeFunc>& getComponentCreationMap() const;\n" +
		"		const std::unordered_map<std::string, uint32>& getComponentTypeMap() const;\n\n" +
		"		const std::unordered_map<uint32, std::string>& getTypeComponentMap() const;\n\n" +
		"	private:\n" +
		"		std::unordered_map<std::string, CreateObjectOfTypeFunc> systemCreationMap_;\n" +
		"		std::unordered_map<std::string, CreateComponentOfTypeFunc> componentCreationMap_;\n" +
		"		std::unordered_map<std::string, uint32> componentTypeMap_;\n" +
		"		std::unordered_map<uint32, std::string> typeComponentMap_;\n" +
		"	}; //class Serializer\n" +
		"} //namespace reflection\n\n";
	return retString;
}

std::string HeaderTool::generateHeaderEnumBeggining() {
	return std::string() +
		"namespace reflection\n" +
		"{\n" +
		"	enum class ComponentEnum\n" +
		"	{\n";
}

std::string HeaderTool::generateHeaderEnumComponent(const std::string& className, const std::string& type) {
	return
		"		" + className + " = " + type + ",\n";
}

std::string HeaderTool::generateHeaderEnumEnding() {
	return std::string() +
		"	}; //enum class ComponentEnum\n" +
		"} //namespace reflection\n";
}

std::string HeaderTool::generateSourceBasicComponent(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	return std::string() +
		"	entity.addComponent<" + typeName + ">();\n";
}

std::string HeaderTool::generateComponentTypeAssignment(const std::string& nmspace, const std::string& className,
	int componentType) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"	" + typeName + "::type = " + to_string(componentType) + ";\n" +
		"	componentTypeMap_.insert({ \"" + typeName + "\", " + to_string(componentType) + " });\n" +
		"	typeComponentMap_.insert({ " + to_string(componentType) + ", \"" + typeName + "\" });\n";
	return retString;
}

std::string HeaderTool::generateMapAssignment(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"	systemCreationMap_.insert({ \"" + typeName + "\", &createObjectOfType<" + typeName + "> });\n";
	return retString;
}

std::string HeaderTool::generateComponentMapAssignment(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"	componentCreationMap_.insert({ \"" + typeName + "\", &createComponentOfType<" + typeName + "> });\n";
	return retString;
}

std::string HeaderTool::generateTemplateFactoryDefinition(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"template<> eecs::SystemBase* createObjectOfType<" + typeName + ">() { return new " + typeName + "(); }\n";
	return retString;
}

std::string HeaderTool::generateComponentFactoryDefinition(const std::string& nmspace, const std::string& className) {
	const std::string typeName = nmspace + "::" + className;
	std::string retString = std::string() +
		"template<> eecs::ComponentBase* createComponentOfType<" + typeName + ">() { return new " + typeName + "(); }\n";
	return retString;
}

std::string HeaderTool::generateSourceBeggining() {
	std::string retString = std::string() +
		"//File generated by Flavo Header Tool. DO NOT edit unless you are sure you know what to do\n" +
		"#include \"Serializer.h\"\n\n" +
		"reflection::Serializer::Serializer() {\n";
	return retString;
}

std::string HeaderTool::generateSourceEnding() {
	std::string retString = std::string() +
		"} //Serializer()\n\n" +
		"reflection::Serializer::~Serializer() { }\n\n" +
		"const std::unordered_map<std::string, CreateObjectOfTypeFunc>& reflection::Serializer::getSystemCreationMap() const {\n" +
		"	return systemCreationMap_;\n" +
		"}\n\n" +
		"const std::unordered_map<std::string, CreateComponentOfTypeFunc>& reflection::Serializer::getComponentCreationMap() const {\n" +
		"	return componentCreationMap_;\n" +
		"}\n\n" +
		"const std::unordered_map<std::string, uint32>& reflection::Serializer::getComponentTypeMap() const {\n" +
		"	return componentTypeMap_;\n" +
		"}\n\n" +
		"const std::unordered_map<uint32, std::string>& reflection::Serializer::getTypeComponentMap() const {\n" +
		"	return typeComponentMap_;\n" +
		"}\n\n" +
		"void reflection::Serializer::initRootComponent(eecs::Entity entity) const {\n";
	return retString;
}

std::string HeaderTool::generateSourceEnding2() {
	std::string retString = std::string() +
		"}\n\n";
	return retString;
}
