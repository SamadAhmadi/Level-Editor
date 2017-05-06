// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/glew.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <Windows.h>
#include <tinyxml2.h>

#include "General/Game.h"
#include "Rendering\AssimpLoader.h"


std::vector<std::string> ListDirectoryContents(const char *sDir)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[2048];
	std::vector<std::string> paths;

	//Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", sDir);
		return paths;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

			//Is the entity a File or Folder?
			if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
			{
				printf("Directory: %s\n", sPath);
				paths.push_back(sPath);
				std::vector<std::string> temp = ListDirectoryContents(sPath);//Recursion, I love it!
				paths.insert(paths.end(), temp.begin(), temp.end()); 

			}
			else {
				printf("File: %s\n", sPath);
				paths.push_back(sPath);

			}
		}
	} while (FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return paths;
}

std::vector<std::string> filesNames(std::string folder)
{
	std::vector<std::string> fileNames;
	std::string search_path = folder + "/*.*";
	WIN32_FIND_DATA find;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &find);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				fileNames.push_back(find.cFileName);
			}
		} while (::FindNextFile(hFind, &find));
		::FindClose(hFind);
	}
	return fileNames;
}

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

template <typename T>
void remove_duplicates(std::vector<T>& vec)
{
	std::sort(vec.begin(), vec.end());
	vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	
	WindowManager::getInstance().createWindow("Level Editor", 0, 0, 1920, 1080);
	GLFWwindow* window = WindowManager::getInstance().getWindow();
    glfwMakeContextCurrent(window);
    glewInit();

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

	Scene * _Scene = new Scene();

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = false;
    bool AssetsWindow = false;
	bool ModelWindow = true;
	bool ScriptWindow = true;

	ImVec4 clear_color = ImColor(92, 8, 135);
	//pre set component
	std::vector<bool> componentToggle(9);
	componentToggle[0] = true;
	//script component
	std::vector<bool> scriptComponentToggle(100);
	scriptComponentToggle[0] = false;
	//models in scene
	std::vector<bool> ModelToggle(150);
	ModelToggle[0] = false;

	//script
	std::vector<std::string> fileNameBackup = ListDirectoryContents("./Script"); // back up of the names stored
	std::vector<const char *> comboStrings; // names of script
	comboStrings.push_back("Temp");
	//xml
	std::vector<std::string> XMLNameBackup = ListDirectoryContents("./XML"); // back up of the xml stored
	std::vector<const char *> XMLcomboStrings; // names of xml
	XMLcomboStrings.push_back("XMLTemp");

	//model
	std::vector<std::string> ModelsNameBackup = ListDirectoryContents("./Models"); // back up of the models stored
	std::vector<const char *> ModelscomboStrings; // names of model	
	ModelscomboStrings.push_back("ModelTemp");
	//texture
	std::vector<std::string> TexturesNameBackup = ListDirectoryContents("./Textures"); // back up of the texture stored
	std::vector<const char *> TexturescomboStrings; // names of texture

	ResourceManager::getInstance()->LoadShader("./Shaders/texture_shader.vert", "./Shaders/texture_shader.frag", "default");

	WindowManager::getInstance().getSceneManager()->LoadScene(_Scene);
	Renderer * renderer =  new Renderer(WindowManager::getInstance().getWindow());
	GUIRenderer * gRenderer = new GUIRenderer(WindowManager::getInstance().getWindow());

	WindowManager::getInstance().getSceneManager()->switchScene();
	WindowManager::getInstance().getSceneManager()->UpdateRenderers(renderer, gRenderer);
	_Scene = WindowManager::getInstance().getSceneManager()->getCurrentScene();


	DirectionalLight * dirLight = new DirectionalLight("default", glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f, 0.05f, 0.1f), glm::vec3(0.2f, 0.2f, 0.7f), glm::vec3(0.7f, 0.7f, 0.7f));

	renderer->getLightManager().RegisterDirectionalLight(dirLight);

	GameObject object("model");
	int index = _Scene->AddGameObject(object);
	TransformComponent * tc = _Scene->getGameObjects()->at(index).GetComponentByType<TransformComponent>();
	tc->setParent(&_Scene->getGameObjects()->at(index));
	//tc->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	//tc->setRotation(glm::vec3(-3.142 / 2.0f, 0.0f, 0.0f));
	//tc->setScale(glm::vec3(2.0f, 2.0f, 2.0f));

	RenderComponent * render = new RenderComponent(&_Scene->getGameObjects()->at(index), "default");

	AssimpLoader loader;

	Model model = loader.LoadModel("./Models/nanosuit/nanosuit.obj");
	render->AttachModel(model);
	_Scene->getGameObjects()->at(index).registerComponent(render);
	render->setParent(&_Scene->getGameObjects()->at(index));

	ResourceManager::getInstance()->useShader("default");

	glm::vec3 pointLightPositions[] = 
	{
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glm::vec3 pointLightColors[] = 
	{

		glm::vec3(0.7f, 0.7f, 0.7f),
		glm::vec3(0.7f, 0.7f, 0.7f),
		glm::vec3(0.7f, 0.7f, 0.7f),
		glm::vec3(0.7f, 0.7f, 0.7f)
	};


	for (int i = 0; i < 4; i++) 
	{
		ShaderUniform position;
		position.M_Address = "pointLights[" + to_string(i) + "].position";
		position.M_Type = ShaderType::VEC3;
		position.M_Vec3 = glm::vec3(pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);

		ShaderUniform ambient;
		ambient.M_Address = "pointLights[" + to_string(i) + "].ambient";
		ambient.M_Type = ShaderType::VEC3;
		ambient.M_Vec3 = glm::vec3(pointLightColors[i].x * 0.1, pointLightColors[i].y * 0.1, pointLightColors[i].z * 0.1);

		ShaderUniform diffuse;
		diffuse.M_Address = "pointLights[" + to_string(i) + "].diffuse";
		diffuse.M_Type = ShaderType::VEC3;
		diffuse.M_Vec3 = glm::vec3(pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z);

		ShaderUniform specular;
		specular.M_Address = "pointLights[" + to_string(i) + "].specular";
		specular.M_Type = ShaderType::VEC3;
		specular.M_Vec3 = glm::vec3(pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z);

		ShaderUniform constant;
		constant.M_Address = "pointLights[" + to_string(i) + "].constant";
		constant.M_Type = ShaderType::FLOAT;
		constant.M_Float = 1.0f; 

		ShaderUniform linear;
		linear.M_Address = "pointLights[" + to_string(i) + "].linear";
		linear.M_Type = ShaderType::FLOAT;
		linear.M_Float = 0.09f;

		ShaderUniform quadratic;
		quadratic.M_Address = "pointLights[" + to_string(i) + "].quadratic";
		quadratic.M_Type = ShaderType::FLOAT;
		quadratic.M_Float = 0.032f;

		PointLight * pointLight = new PointLight("default", glm::vec3(pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z), glm::vec3(pointLightColors[i].x * 0.1, pointLightColors[i].y * 0.1, pointLightColors[i].z * 0.1), glm::vec3(pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z), glm::vec3(pointLightColors[i].x, pointLightColors[i].y, pointLightColors[i].z), 1.0f, 0.09f, 0.032f);


		renderer->getLightManager().RegisterPointLight(pointLight);

	}

	if (WindowManager::getInstance().getSceneManager()->getCurrentScene() != nullptr)
		WindowManager::getInstance().getSceneManager()->getCurrentScene()->Start();

	//Model
	static float vec3mPos[3] = {0.0f, 0.0f, 0.0f };
	static float vec3mRot[3] = { 0.0f, 0.0f, 0.0f };
	static float vec3mSize[3] = { 1.0f, 1.0f, 1.0f };
	//Camera
	static float vec3cPos[3] = { 0.0f, 0.0f, 0.0f };
	static float vec3cRot[3] = { 0.0f, 0.0f, 0.0f };
	//Directional Light
	static float vec3DLPos[3] = { -0.2f, -1.0f, -0.3f };
	static float vec3DLA[3] = { 0.05f, 0.05f, 0.1f };
	static float vec3DLD[3] = { 0.2f, 0.2f, 0.7f };
	static float vec3DLS[3] = { 0.7f, 0.7f, 0.7f };
	//Point Lights
	static float vec3PointPos[3] = { -0.2f, -1.0f, -0.3f };
	static float vec3PointA[3] = { 0.05f, 0.05f, 0.1f };
	static float vec3PointD[3] = { 0.2f, 0.2f, 0.7f };
	static float vec3PointS[3] = { 0.7f, 0.7f, 0.7f };
	static float Constant = 0.0f;
	static float Linear = 0.0f;
	static float Quadratic = 0.0f;


	// Main loop
while (!glfwWindowShouldClose(window))
{
	glfwPollEvents();
	ImGui_ImplGlfwGL3_NewFrame();

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	/*glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);*/



	_Scene->Update(0);
	renderer->update(0);
	renderer->Render();

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
		ImGui::SetWindowSize(ImVec2(1920, 200), ImGuiSetCond_Always);
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Window1")) show_test_window ^= 1;
		if (ImGui::Button("Assets")) AssetsWindow ^= 1;
		if (ImGui::Button("Model")) ModelWindow ^= 1;
		if (ImGui::Button("Script")) ScriptWindow ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// assets
	if (AssetsWindow)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Assets", &AssetsWindow);
		ImGui::Text("Hello");
		// list of models

		static int item = -1;
		//remove_duplicates
			if (ModelsNameBackup.size() > 0)
			{
				ModelscomboStrings.clear();
				remove_duplicates(ModelsNameBackup);
				for (int i = 0; i < ModelsNameBackup.size(); i++)
				{
					ModelscomboStrings.push_back(const_cast<char *>(ModelsNameBackup[i].c_str()));
					//scriptComponentToggle.resize(comboStrings.size() * 2);
					if (ModelscomboStrings.capacity() > ModelToggle.capacity())
					{
						ModelToggle.resize(ModelscomboStrings.capacity());
					}
				}
			}
			const char** cItem = &ModelscomboStrings[0];

		if (ImGui::TreeNode("Models"))
		{
			ImGui::Combo("Model List", &item, cItem, ModelscomboStrings.size());
			ImGui::TreePop();
		}
		//list of models in scene
		if (ImGui::TreeNode("Models in scene"))
		{
			ImGui::Columns(3, NULL, false);
			for (int i = 0; i < ModelscomboStrings.size(); i++)
			{
				if (ImGui::Selectable(ModelscomboStrings[i], ModelToggle[i]))
				{
					ModelToggle[i] = !ModelToggle[i];
				}
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::TreePop();
		}
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
	if (ScriptWindow)
	{
		ImGui::SetNextWindowSize(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Script", &ScriptWindow);
		//Scripting

		static bool read_only = false; //read only
		static char text[1024 * 32]; // text box
		static char title[128]; // title
		std::ofstream SaveFile; // save script
		std::ifstream LoadFile;	// load script			
		if (ImGui::Button("New Script"))
		{
			for (int i = 0; i < 1024 * 32; i++)
			{
				text[i] = 0; // clear
			}
			for (int i = 0; i < 128; i++)
			{
				title[i] = 0; //clear
			}
		};
		ImGui::InputText("Script Title", title, IM_ARRAYSIZE(title));

		static int item = -1;

		//ImGui::Combo("combo", &item2, "aaaa\0bbbb\0cccc\0dddd\0eeee\0\0");
		//comboStrings.insert(comboStrings.end(), title, title + strlen(title));

		//remove_duplicates
		if (fileNameBackup.size() > 0) {
			comboStrings.clear();
			remove_duplicates(fileNameBackup);
			for (int i = 0; i < fileNameBackup.size(); i++) {
				comboStrings.push_back(const_cast<char *>(fileNameBackup[i].c_str()));
				//scriptComponentToggle.resize(comboStrings.size() * 2);
				if (comboStrings.capacity() > scriptComponentToggle.capacity())
					scriptComponentToggle.resize(comboStrings.capacity());
			}
		}
		const char** cItem = &comboStrings[0];

		ImGui::Combo("Script List", &item, cItem, comboStrings.size());

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::Checkbox("Read-only", &read_only);
		ImGui::PopStyleVar();
		ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 50), ImGuiInputTextFlags_AllowTabInput | (read_only ? ImGuiInputTextFlags_ReadOnly : 0));
		if (ImGui::Button("Load Script"))
		{
				for (int i = 0; i < 1024 * 32; i++)
				{
					text[i] = 0; // clear
				}

					std::string scriptPath = std::string(comboStrings[item]);
					if (!strstr(scriptPath.c_str(), "./Script")) {
						scriptPath = "./Script/" + scriptPath;
					}
					LoadFile.open(scriptPath);
					if (LoadFile.is_open())
					{
						for (int i = 0; i < 1024 * 32; i++)
						{
							LoadFile >> std::noskipws >> text[i];
						}
					}
					LoadFile.close();
		};
		if (ImGui::Button("Save Script"))
		{
			fileNameBackup.push_back(std::string(title) + ".lua");
			comboStrings.push_back(fileNameBackup[fileNameBackup.size() - 1].c_str());
			SaveFile.open("./Script/" + std::string(title) + ".lua");
			SaveFile << text;
			SaveFile.close();
		};

		ImGui::End();
	}
	// right side menu system
	if (ModelWindow)
	{
		//xml
		std::ofstream SaveXML;
		std::ifstream LoadXML;
		static char XMLsize[1024 * 32]; // text box
		static char title[128]; // title
		const char** cItem = &XMLcomboStrings[0];
		static int item = -1;

		static float V = 0.0f;

		static bool Show = true;
		//transform is alway true unless changed

		ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Window", &ModelWindow);

		//remove_duplicates
		if (XMLNameBackup.size() > 0) {
			XMLcomboStrings.clear();
			remove_duplicates(XMLNameBackup);
			for (int i = 0; i < XMLNameBackup.size(); i++) {
				XMLcomboStrings.push_back(const_cast<char *>(XMLNameBackup[i].c_str()));
			}
		}

		//Options
		if (ImGui::TreeNode("Options"))
		{
			if (ImGui::Button("New XML"))
			{
				for (int i = 0; i < 1024 * 32; i++)
				{
					XMLsize[i] = 0; // clear
				}
				for (int i = 0; i < 128; i++)
				{
					title[i] = 0; //clear
				}
			}
			if (ImGui::TreeNode("Load XML"))
			{
				for (int i = 0; i < 1024 * 32; i++)
				{
					XMLsize[i] = 0; // clear
				}
				ImGui::Combo("XML List", &item, cItem, XMLcomboStrings.size());
				if (ImGui::Button("Load XML"))
				{
					LoadXML.open("./XML/" + std::string(XMLcomboStrings[item]));
					if (LoadXML.is_open())
					{
						for (int i = 0; i < 1024 * 32; i++)
						{
							LoadXML >> std::noskipws >> XMLsize[i];
						}
					}
					LoadXML.close();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Save As XML"))
			{
				ImGui::InputText("XML Title", title, IM_ARRAYSIZE(title));
				if (ImGui::Button("Save XML"))
				{
					XMLNameBackup.push_back(std::string(title) + ".xml");
					comboStrings.push_back(XMLNameBackup[XMLNameBackup.size() - 1].c_str());
					SaveXML.open("./XML/" + std::string(title) + ".xml");
					SaveXML << XMLsize;
					SaveXML.close();
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		ImGui::Text(" ");
		ImGui::Text("Player Model");

		//Component
		if (ImGui::TreeNode("Add Component"))
		{
			ImGui::Columns(3, NULL, false);
			std::string Components[] = { "Transform", "Camera", "Audio", "Directional Light", "Point Lights", "templateComponents2", "templateComponents3", "templateComponents4", "templateComponents5" };
			for (int i = 0; i < 9; i++)
			{
				if (ImGui::Selectable(Components[i].c_str(), componentToggle[i]))
				{
					componentToggle[i] = !componentToggle[i];
				}
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::TreePop();
		}
		if (componentToggle[0] == true)
		{
			if (ImGui::TreeNode("Transform"))
			{
				ImGui::InputFloat3("Position", vec3mPos);
				tc->setPosition(glm::vec3(vec3mPos[0], vec3mPos[1], vec3mPos[2]));
				ImGui::InputFloat3("Rotation", vec3mRot);
				tc->setRotation(glm::vec3(vec3mRot[0], vec3mRot[1], vec3mRot[2]));
				ImGui::InputFloat3("Scale", vec3mSize);
				tc->setScale(glm::vec3(vec3mSize[0], vec3mSize[1], vec3mSize[2]));
				ImGui::TreePop();
			}
		}
		if (componentToggle[1] == true)
		{
			if (ImGui::TreeNode("Camera"))
			{
				ImGui::InputFloat3("Position", vec3cPos);
				ImGui::InputFloat3("Rotation", vec3cRot);
				ImGui::TreePop();
			}
		}
		if (componentToggle[2] == true)
		{
			if (ImGui::TreeNode("Audio"))
			{
				ImGui::VSliderFloat("Volume", ImVec2(50, 400), &V, 0.0f, 100.0f, "%.0f");
				ImGui::TreePop();
			}
		}
		if (componentToggle[3] == true)
		{
			if (ImGui::TreeNode("Directional Light"))
			{
				ImGui::InputFloat3("Direction", vec3DLPos);
				ImGui::InputFloat3("Ambient", vec3DLA);
				ImGui::InputFloat3("Diffuse", vec3DLD);
				ImGui::InputFloat3("Specular", vec3DLS);
				ImGui::TreePop();
			}
		}
		if (componentToggle[4] == true)
		{
			if (ImGui::TreeNode("Point Lights"))
			{
				ImGui::InputFloat3("Direction", vec3PointPos);
				ImGui::InputFloat3("Ambient", vec3PointA);
				ImGui::InputFloat3("Diffuse", vec3PointD);
				ImGui::InputFloat3("Specular", vec3PointS);
				//ImGui::SliderFloat("Constant", Constant);
				//ImGui::SliderFloat("Linear", Linear);
				//ImGui::SliderFloat("Quadratic", Quadratic);
				ImGui::TreePop();
			}
		}

		//Component script
		if (ImGui::TreeNode("Add Component Script"))
		{
			ImGui::Columns(3, NULL, false);
			for (int i = 0; i < comboStrings.size(); i++)
			{
				if (ImGui::Selectable(comboStrings[i], scriptComponentToggle[i]))
				{
					scriptComponentToggle[i] = !scriptComponentToggle[i];
				}
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::TreePop();
		}

		ImGui::Text(" ");
		ImGui::Checkbox("Show Model", &Show);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
