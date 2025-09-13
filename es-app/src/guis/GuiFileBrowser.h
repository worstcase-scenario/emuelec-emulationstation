#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "ApiSystem.h"

template<typename T>
class OptionListComponent;

class ImageComponent;
#include <vector>

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

        GuiFileBrowser(Window* window, const std::string startPath, const std::string selectedFile, FileTypes types = FileTypes::IMAGES, const std::function<void(const std::string&)>& okCallback = nullptr, const std::string& title = "");
        ~GuiFileBrowser() override;

        bool input(InputConfig* config, Input input) override;
        void update(int deltaTime) override;
        virtual std::vector<HelpPrompt> getHelpPrompts() override;

private:
        void onOk(const std::string& path);
        void navigateTo(const std::string path);
        void centerWindow();
        void generateVideoPreview(const std::string& path);
        void clearVideoPreview();

        MenuComponent mMenu;

        std::string mCurrentPath;
        std::string mSelectedFile;
        FileTypes   mTypes;
        std::shared_ptr<ImageComponent> mPreview;
        std::vector<std::string> mVideoFrames;
        int mCurrentFrame;
        int mFrameTime;
        std::string mTempPreviewDir;
       bool mGeneratingPreview;
       int mExpectedFrames;

        std::function<void(const std::string&)> mOkCallback;
};
