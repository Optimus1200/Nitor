#include "Gui.h"
#include <iostream>
#include <imgui_internal.h>

namespace ntr
{
	void Gui::init(GLFWwindow* window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init();
	}
	
	void Gui::terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	Gui::EditableSelectableResult Gui::EditableSelectable(const std::string& label, bool selected)
	{
		static ImGuiID renamingID = 0;
		static char renameBuffer[256] = { '\0' };
		static bool showError = false;
		static std::string errorMessage;

		EditableSelectableResult result;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiID id = window->GetID(label.c_str());

		bool isRenaming = (renamingID == id);

		if (isRenaming)
		{
			result.shouldSelect = true;

			// Initialize buffer when rename starts
			if (ImGui::IsItemActivated())
			{
				strncpy_s(renameBuffer, label.c_str(), sizeof(renameBuffer) - 1);
				renameBuffer[sizeof(renameBuffer) - 1] = '\0';
				showError = false;
			}

			// Input with validation
			bool enterPressed = ImGui::InputText("##Rename", renameBuffer, sizeof(renameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue |
				ImGuiInputTextFlags_AutoSelectAll |
				ImGuiInputTextFlags_CharsNoBlank);

			// Validate on enter or when trying to exit
			if (enterPressed || (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)))
			{
				std::string newName(renameBuffer);

				// Trim whitespace
				newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);
				newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));

				if (newName.empty())
				{
					showError = true;
					errorMessage = "Name cannot be empty!";
				}
				else if (newName.find_first_of("#@!?") != std::string::npos)
				{
					showError = true;
					errorMessage = "Invalid characters detected! (#, @, !, ?)";
				}
				else
				{
					result.shouldRename = true;
					result.newName = newName;
					renamingID = 0;
					showError = false;
				}
			}

			// Show error tooltip if validation failed
			if (showError && ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				ImGui::Text("%s", errorMessage.c_str());
				ImGui::PopStyleColor();
				ImGui::EndTooltip();
			}

			ImGui::SetKeyboardFocusHere(-1);

			// Only allow escape to cancel
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				result.shouldRename = false;
				renamingID = 0;
				showError = false;
			}
		}
		else
		{
			if (ImGui::Selectable(label.c_str(), selected))
			{
				result.shouldSelect = true;
			}

			if (ImGui::BeginPopupContextItem())
			{
				result.shouldSelect = true;
				if (ImGui::MenuItem("Delete"))
					result.shouldDelete = true;
				ImGui::EndPopup();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				result.shouldSelect = true;
				strncpy_s(renameBuffer, label.c_str(), sizeof(renameBuffer) - 1);
				renameBuffer[sizeof(renameBuffer) - 1] = '\0';
				renamingID = id;
			}
		}

		return result;
	}

	Gui::EditableTreeNodeResult Gui::EditableTreeNode(const char* str_id, bool selected)
	{
		EditableTreeNodeResult treeNodeResult;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiID id = window->GetID(str_id);

		// Arrow button (toggle state)
		treeNodeResult.isOpen = ImGui::GetStateStorage()->GetBool(id, false);
		ImGuiDir arrowDir = treeNodeResult.isOpen ? ImGuiDir_Down : ImGuiDir_Right;

		ImGui::PushID("TreeNodeArrowButton");
		if (ImGui::ArrowButton(str_id, arrowDir))
		{
			treeNodeResult.isOpen = !treeNodeResult.isOpen;
			ImGui::GetStateStorage()->SetBool(id, treeNodeResult.isOpen);  // Save state
		}
		ImGui::PopID();

		ImGui::SameLine();

		EditableSelectableResult selectableResult = EditableSelectable(str_id, selected);

		treeNodeResult.shouldSelect	= selectableResult.shouldSelect;
		treeNodeResult.shouldRename	= selectableResult.shouldRename;
		treeNodeResult.shouldDelete	= selectableResult.shouldDelete;
		treeNodeResult.newName		= selectableResult.newName;

		return treeNodeResult;
	}

	void Gui::showErrorTooltip(const std::string& message)
	{
		ImGui::BeginTooltip();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
		ImGui::Text("%s", message.c_str());
		ImGui::PopStyleColor();
		ImGui::EndTooltip();
	}

	void Gui::clear()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Gui::draw()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	
	FileExplorer::FileExplorer(const std::string& title)
		: mTitle{ title }
		, mOpenButtonTitle{ "Open" }
		, mCurrentPath{ std::filesystem::current_path() }
		, mCurrentPathBuffer{ "\0" }
		, mSearchBuffer{ "\0" }
		, mIsOpen{ false }
	{
	}

	void FileExplorer::close()
	{
		mIsOpen = false;
	}

	void FileExplorer::filterFileTypes(std::initializer_list<std::string> filetypes)
	{
		mFiletypeFilter = filetypes;
	}

	const std::unordered_set<std::filesystem::path>& FileExplorer::getSelectedFiles() const
	{
		return mSelectedFiles;
	}

	bool FileExplorer::isOpen() const
	{
		return mIsOpen;
	}

	void FileExplorer::open()
	{
		mIsOpen = true;
	}

	void FileExplorer::render()
	{
		ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

		ImGui::Begin(mTitle.c_str(), &mIsOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

		strcpy_s(mCurrentPathBuffer, mCurrentPath.string().c_str());

		float headerHeight = 20.0f;
		float footerHeight = 30.0f;
		float bodyHeight = ImGui::GetContentRegionAvail().y - headerHeight - footerHeight;

		// Header

		ImGui::BeginChild("##Header", ImVec2(0, headerHeight), false);

		if (ImGui::Button("<"))
		{
			mCurrentPath = mCurrentPath.parent_path();
			strcpy_s(mCurrentPathBuffer, mCurrentPath.string().c_str());
			mSearchBuffer[0] = '\0';
		}

		ImGui::SameLine();

		if (ImGui::InputText("##CurrentDir", mCurrentPathBuffer, sizeof(mCurrentPathBuffer)))
		{
			if (std::filesystem::exists(mCurrentPathBuffer))
			{
				mCurrentPath = std::filesystem::path(mCurrentPathBuffer);
			}
		}
		
		ImGui::SameLine();

		ImGui::InputTextWithHint("##Search", "Search...", mSearchBuffer, sizeof(mSearchBuffer));

		ImGui::EndChild();

		// Body (scrollable file list)

		ImGui::BeginChild("##Body", ImVec2(0, bodyHeight), true);

		const auto SORTED_ENTRIES = getSortedDirectoryEntries(mCurrentPath);

		for (const auto& entry : SORTED_ENTRIES)
		{
			std::string entryName = entry.path().filename().string();

			if (std::filesystem::is_directory(entry))
			{
				entryName += "/";
			}

			bool isSelected = mSelectedFiles.find(entry.path()) != mSelectedFiles.end();

			if (ImGui::Selectable(entryName.c_str(), isSelected))
			{
				if (std::filesystem::is_directory(entry))
				{
					mCurrentPath = entry;
					strcpy_s(mCurrentPathBuffer, mCurrentPath.string().c_str());
					mSelectedFiles.clear();
					mSearchBuffer[0] = '\0';
					break; // changing directories
				}
				else
				{
					if (isSelected)
					{
						mSelectedFiles.erase(entry.path());
					}
					else
					{
						mSelectedFiles.emplace(entry.path());
					}
				}
			}
		}

		ImGui::EndChild();

		// Footer

		ImGui::BeginChild("##Footer", ImVec2(0, footerHeight), false);

		float buttonWidth = 100.0f;
		ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - buttonWidth);
		if (ImGui::Button(mOpenButtonTitle.c_str(), ImVec2(buttonWidth, 0)))
		{
			mOnFileOpenCallback();
			mSelectedFiles.clear();
			mCurrentPath = std::filesystem::current_path();
			mIsOpen = false;
		}

		ImGui::EndChild();

		ImGui::End();
	}

	void FileExplorer::setOpenButtonTitle(const std::string& title)
	{
		mOpenButtonTitle = title;
	}

	void FileExplorer::setTitle(const std::string& title)
	{
		mTitle = title;
	}

	void FileExplorer::setOnFileOpenCallback(const std::function<void()>& callback)
	{
		mOnFileOpenCallback = callback;
	}
	
	std::vector<std::filesystem::directory_entry> FileExplorer::getSortedDirectoryEntries(const std::filesystem::path& dirPath) const
	{
		std::vector<std::filesystem::directory_entry> entries;

		if (!std::filesystem::exists(dirPath))
		{
			return entries;
		}

		// Convert search term to lowercase for case-insensitive comparison
		std::string searchTerm = mSearchBuffer;
		std::transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(),
			[](unsigned char c) { return std::tolower(c); });

		bool fileFilterDisabled = mFiletypeFilter.empty();

		for (const auto& entry : std::filesystem::directory_iterator(dirPath))
		{
			// Get filename without full path for searching
			std::string filename = entry.path().filename().string();
			std::string lowercaseFilename = filename;
			std::transform(lowercaseFilename.begin(), lowercaseFilename.end(), lowercaseFilename.begin(),
				[](unsigned char c) { return std::tolower(c); });

			// Check if filename starts with search term (case-insensitive)
			bool matchesSearch = searchTerm.empty() || lowercaseFilename.rfind(searchTerm, 0) == 0;

			if (!matchesSearch) {
				continue;
			}

			if (std::filesystem::is_directory(entry))
			{
				entries.emplace_back(entry);
			}
			else if (fileFilterDisabled)
			{
				entries.emplace_back(entry);
			}
			else
			{
				// Check file extension against filter
				size_t dotIndex = filename.find_last_of('.');
				if (dotIndex != std::string::npos)
				{
					std::string filetype = filename.substr(dotIndex + 1);
					std::transform(filetype.begin(), filetype.end(), filetype.begin(),
						[](unsigned char c) { return std::tolower(c); });

					if (mFiletypeFilter.find(filetype) != mFiletypeFilter.end())
					{
						entries.emplace_back(entry);
					}
				}
			}
		}

		// Sort entries with directories first, then by filename
		std::sort(entries.begin(), entries.end(),
			[](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				// Directories come before files
				if (a.is_directory() != b.is_directory()) {
					return a.is_directory() > b.is_directory();
				}

				// Case-insensitive filename comparison
				std::string aName = a.path().filename().string();
				std::string bName = b.path().filename().string();
				std::transform(aName.begin(), aName.end(), aName.begin(), ::tolower);
				std::transform(bName.begin(), bName.end(), bName.begin(), ::tolower);

				return aName < bName;
			});

		return entries;
	}
}