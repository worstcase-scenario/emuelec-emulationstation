#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "ApiSystem.h"
#include "components/ImageComponent.h"
#include <memory>

template<typename T>
class OptionListComponent;

class GuiFileBrowser : public GuiComponent
{
public:
	enum FileTypes
	{
		IMAGES = 1,
		MANUALS = 2,
		VIDEO = 3,
		DIRECTORY = 4,
		AUDIO = 5,
		ALL = 255
	};

GuiFileBrowser(Window* window, const std::string startPath, const std::string selectedFile, FileTypes types = FileTypes::IMAGES, const std::function<void(const std::string&)>& okCallback = nullptr, const std::string& title = "", bool showPreview = false);
~GuiFileBrowser() override;

bool input(InputConfig* config, Input input) override;
virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void onOk(const std::string& path);
	void navigateTo(const std::string path);
	void centerWindow();

        MenuComponent mMenu;
        std::unique_ptr<ImageComponent> mPreview;

        std::string mCurrentPath;
        std::string mSelectedFile;
        FileTypes   mTypes;

        std::function<void(const std::string&)> mOkCallback;
};