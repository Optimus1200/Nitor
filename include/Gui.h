#ifndef NTR_GUI_H
#define NTR_GUI_H

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_set>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace ntr
{
	class Gui
	{
	public:

		struct EditableSelectableResult
		{
			bool shouldSelect = false;
			bool shouldRename = false;
			bool shouldDelete = false;
			std::string newName = "";
			
		};

		struct EditableTreeNodeResult
		{
			bool shouldSelect = false;
			bool shouldRename = false;
			bool shouldDelete = false;
			std::string newName = "";
			bool isOpen = false;
		};
		
		static void init(GLFWwindow* window);
		static void terminate();

		// 1. Rename - (Double Left-Click) Turns the Selectable into an editable InputText, result is stored in EditableSelectableResult.shouldRename (bool) and EditableSelectableResult.newName (string)
		// 2. Delete - (Right-Click) Opens a context menu, left-clicking the Delete option marks the Selectable for deletion, result is stored in EditableSelectable.shouldDelete
		static EditableSelectableResult EditableSelectable(const std::string& label, bool selected = false);

		// Arrow Button followed by an EditableSelectable with options:
		// 1. Rename - (Double Left-Click) Turns the Selectable into an editable InputText, result is stored in EditableSelectableResult.shouldRename (bool) and EditableSelectableResult.newName (string)
		// 2. Delete - (Right-Click) Opens a context menu, left-clicking the Delete option marks the Selectable for deletion, result is stored in EditableSelectable.shouldDelete
		static Gui::EditableTreeNodeResult EditableTreeNode(const char* str_id, bool selected = false);

		static void showErrorTooltip(const std::string& message);

		static void clear();
		static void draw();
	};

	class FileExplorer
	{
	public:

		FileExplorer(const std::string& title = "File Explorer");

		void close();
		void filterFileTypes(std::initializer_list<std::string> filetypes);
		const std::unordered_set<std::filesystem::path>& getSelectedFiles() const;
		bool isOpen() const;
		void open();
		void render();
		void setOpenButtonTitle(const std::string& title);
		void setTitle(const std::string& title);
		void setOnFileOpenCallback(const std::function<void()>& callback);

	private:

		std::string											mTitle;
		std::string											mOpenButtonTitle;
		std::filesystem::path								mCurrentPath;
		char												mCurrentPathBuffer[256];
		char												mSearchBuffer[256];
		std::unordered_set<std::filesystem::path>			mSelectedFiles;
		std::unordered_set<std::string>						mFiletypeFilter; // show only files of certain types (jpg, png, etc) 
		std::function<void()>								mOnFileOpenCallback; // triggered when the "Open" button is clicked
		bool												mIsOpen;

		std::vector<std::filesystem::directory_entry>		getSortedDirectoryEntries(const std::filesystem::path& path) const;
	};
}

#endif