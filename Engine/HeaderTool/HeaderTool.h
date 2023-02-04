#pragma once

#include <string>
#include "Logger.h"

class HeaderTool
{
public:
	void generate();

private:
	static std::string generateSourceInclude(const std::string& path);

	static std::string generateTemplateFactoryString(const std::string& nmspace, const std::string& className);
	static std::string generateComponentFactoryString(const std::string& nmspace, const std::string& className);
	static std::string generateHeaderBeggining();
	static std::string generateHeaderEnding();

	static std::string generateHeaderEnumBeggining();
	static std::string generateHeaderEnumComponent(const std::string& className, const std::string& type);
	static std::string generateHeaderEnumEnding();

	static std::string generateSourceBasicComponent(const std::string& nmspace, const std::string& className);
	static std::string generateComponentTypeAssignment(const std::string& nmspace, const std::string& className, int componentType);
	static std::string generateMapAssignment(const std::string& nmspace, const std::string& className);
	static std::string generateComponentMapAssignment(const std::string& nmspace, const std::string& className);
	static std::string generateTemplateFactoryDefinition(const std::string& nmspace, const std::string& className);
	static std::string generateComponentFactoryDefinition(const std::string& nmspace, const std::string& className);
	static std::string generateSourceBeggining();
	static std::string generateSourceEnding();
	static std::string generateSourceEnding2();
};
