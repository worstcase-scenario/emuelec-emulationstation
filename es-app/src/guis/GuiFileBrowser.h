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
		IMAGES = 1 << 0,
		MANUALS = 1 << 1,
		VIDEO = 1 << 2,
		DIRECTORY = 1 << 3,
		AUDIO = 1 << 4,
		ALL = 0x1F
	};

        GuiFileBrowser(Window* window, const std::string startPath, const std::string selectedFile, FileTypes types = FileTypes::IMAGES, const std::function<void(const std::string&)>& okCallback = nullptr, const std::string& title = "", bool showPreview = false);

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