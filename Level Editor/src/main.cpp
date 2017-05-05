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
			if (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
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
	
	WindowManager::getInstance().createWindow("Level Editor Test", 0, 0, 1920, 1080);
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

    bool show_test_window = true;
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

	//script
	std::vector<std::string> fileNameBackup = filesNames("./Script/"); // back up of the names stored
	std::vector<const char *> comboStrings; // names of script
	comboStrings.push_back("Temp");
	//xml
	std::vector<std::string> XMLNameBackup = filesNames("./XML/"); // back up of the xml stored
	std::vector<const char *> XMLcomboStrings; // names of xml
	XMLcomboStrings.push_back("XMLTemp");

	WindowManager::getInstance().getSceneManager()->LoadScene(_Scene);
	Renderer * renderer =  new Renderer(WindowManager::getInstance().getWindow());
	GUIRenderer * gRenderer = new GUIRenderer(WindowManager::getInstance().getWindow());

	WindowManager::getInstance().getSceneManager()->switchScene();
	WindowManager::getInstance().getSceneManager()->UpdateRenderers(renderer, gRenderer);


	// Main loop
while (!glfwWindowShouldClose(window))
{
	glfwPollEvents();
	ImGui_ImplGlfwGL3_NewFrame();

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);

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

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (AssetsWindow)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Assets", &AssetsWindow);
		ImGui::Text("Hello");
		if (ImGui::TreeNode("Models"))
		{
			const char* listbox_items[] = { "Model1", "Model2", "Model3", "Model4", "Model5", "Model6", "Model7", "Model8", "Model9" };
			static int listbox_item_current = 1;
			ImGui::ListBox("Models", &listbox_item_current, listbox_items, IM_ARRAYSIZE(listbox_items), 10);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Textures"))
		{
			const char* listbox_items[] = { "Texture1", "Texture2", "Texture3", "Texture4", "Texture5", "Texture6", "Texture7", "Texture8", "Texture9" };
			static int listbox_item_current = 1;
			ImGui::ListBox("Textures", &listbox_item_current, listbox_items, IM_ARRAYSIZE(listbox_items), 10);
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
			LoadFile.open("./Script/" + std::string(comboStrings[item]));
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
		//model
		static float vec3mPos[3] = { 0.0f, 0.0f, 0.0f };
		static float vec3mRot[3] = { 0.0f, 0.0f, 0.0f };
		static float vec3mSize[3] = { 1.0f, 1.0f, 1.0f };
		//camera
		static float vec3cPos[3] = { 0.0f, 0.0f, 0.0f };
		static float vec3cRot[3] = { 0.0f, 0.0f, 0.0f };

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
			std::string Components[] = { "Transform", "Camera", "Audio", "Light", "templateComponents1", "templateComponents2", "templateComponents3", "templateComponents4", "templateComponents5" };
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
				ImGui::InputFloat3("Rotation", vec3mRot);
				ImGui::InputFloat3("Scale", vec3mSize);
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
