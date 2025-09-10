#include "guis/GuiFileBrowser.h"

#include "ApiSystem.h"
#include "components/OptionListComponent.h"
#include "guis/GuiSettings.h"
#include "views/ViewController.h"
#include "components/ComponentGrid.h"
#include "SystemData.h"
#include "LocaleES.h"
#include "components/MultiLineMenuEntry.h"
#include "GuiLoading.h"
#include "guis/GuiMsgBox.h"
#include <cstring>
#include "SystemConf.h"
#include "Paths.h"
#include "Settings.h"
#include "utils/StringUtil.h"
#include <map>

#define WINDOW_WIDTH (float)Math::max((int)Renderer::getScreenHeight(), (int)(Renderer::getScreenWidth() * 0.65f))

#define DRIVE_ICON		_U("\uF0A0 ")
#define FOLDER_ICON		_U("\uF07C ")
#define IMAGE_ICON		_U("\uF03E ")
#define VIDEO_ICON		_U("\uF03D ")
#define DOCUMENT_ICON	_U("\uF02D ")
#define AUDIO_ICON _U("\uF028 ")



GuiFileBrowser::GuiFileBrowser(Window* window, const std::string startPath, const std::string selectedFile, FileTypes types, const std::function<void(const std::string&)>& okCallback, const std::string& title)
	: GuiComponent(window), mMenu(window, title.empty() ? _("FILE BROWSER") : title)
{
	setTag("popup");

	mTypes = types;
	mSelectedFile = Utils::FileSystem::getCanonicalPath(selectedFile);
	mOkCallback = okCallback;

	addChild(&mMenu);

	if (mOkCallback != nullptr)
	{
		mMenu.addButton(_("RESET"), "back", [&]
		{
			onOk("");			
		});
	}

    mMenu.addButton(_("BACK"), "back", [&] { delete this; });

	if (startPath.empty() || !Utils::FileSystem::isDirectory(startPath))
	{
		mCurrentPath = Settings::getInstance()->getString("LastFileBrowserFolder");
		if (mCurrentPath.empty() || !Utils::FileSystem::isDirectory(mCurrentPath))
			mCurrentPath = Paths::getScreenShotPath();

		navigateTo(mCurrentPath);
	}
	else
		navigateTo(startPath);
}

void GuiFileBrowser::navigateTo(const std::string path)
{
	mCurrentPath = path;

	auto theme = ThemeData::getMenuTheme();

	mMenu.clear();
	mMenu.setSubTitle(mCurrentPath);

	auto files = Utils::FileSystem::getDirectoryFiles(mCurrentPath);

	if (mCurrentPath != "\\" && mCurrentPath != "/" && !mCurrentPath.empty())
	{
		mMenu.addEntry(FOLDER_ICON + std::string(".."), false, [this]()
		{
			navigateTo(Utils::FileSystem::getParent(mCurrentPath));
		});
	}
	files.sort([](const Utils::FileSystem::FileInfo& file1, const Utils::FileSystem::FileInfo& file2) {
	    auto name1 = Utils::FileSystem::getFileName(file1.path);
	    auto name2 = Utils::FileSystem::getFileName(file2.path);
	    return Utils::String::compareIgnoreCase(name1, name2) < 0;
	});
	for (auto file : files)
	{
		if (!file.directory || file.hidden)
			continue;

		std::string icon = FOLDER_ICON;
		if (Utils::String::endsWith(file.path, ":"))
			icon = DRIVE_ICON;

		bool isSelected = false;

		if (mTypes == FileTypes::DIRECTORY)
			isSelected = (mSelectedFile == file.path);

		mMenu.addEntry(icon + Utils::FileSystem::getFileName(file.path), false, [this, file]()
		{
			navigateTo(Utils::FileSystem::combine(mCurrentPath, Utils::FileSystem::getFileName(file.path)));
		}, "", isSelected, false, file.path, false);
	}

	if (mTypes != FileTypes::DIRECTORY)
	{
		for (auto file : files)
		{
			if (file.directory || file.hidden)
				continue;

			std::string ext = Utils::FileSystem::getExtension(file.path);

			std::string icon;
			
			

			if ((mTypes & FileTypes::IMAGES) == FileTypes::IMAGES)
				if (ext == ".jpg" || ext == ".png" || ext == ".gif" || ext == ".svg")
					icon = IMAGE_ICON;

			if ((mTypes & FileTypes::MANUALS) == FileTypes::MANUALS)
				if (ext == ".pdf" || ext == ".cbz")
					icon = DOCUMENT_ICON;

			if ((mTypes & FileTypes::VIDEO) == FileTypes::VIDEO)
				if (ext == ".mp4" || ext == ".avi" || ext == ".mkv" || ext == ".webm")
					icon = VIDEO_ICON;
				
			if ((mTypes & FileTypes::AUDIO) == FileTypes::AUDIO)
				if (ext == ".ogg" || ext == ".mp3" || ext == ".wav")
					icon = AUDIO_ICON;

			if (icon.empty())
				continue;

			bool isSelected = (mSelectedFile == file.path);

			mMenu.addEntry(icon + Utils::FileSystem::getFileName(file.path), false, 
				[this, file]() { onOk(file.path); }, 
				"", isSelected, false, file.path, false);
		}
	}

	centerWindow();	
}

void GuiFileBrowser::centerWindow()
{
	if (Renderer::ScreenSettings::fullScreenMenus())
		mMenu.setSize(Renderer::getScreenWidth(), Renderer::getScreenHeight());
	else
	{
		mMenu.setSize(mMenu.getSize().x(), Renderer::getScreenHeight() * 0.875f);
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
	}
}

bool GuiFileBrowser::input(InputConfig* config, Input input)
{
	if (GuiComponent::input(config, input))
		return true;

	if (config->isMappedTo("y", input) && input.value)
	{
		if (mCurrentPath != "\\" && mCurrentPath != "/" && !mCurrentPath.empty())
			navigateTo(Utils::FileSystem::getParent(mCurrentPath));

		return true;
	}

	if (input.value != 0 && (config->isMappedLike("left", input) || config->isMappedLike("right", input)))
	{
		mMenu.setCursorToButtons();
		return true;
	}

	if (input.value != 0 && config->isMappedTo(BUTTON_BACK, input))
	{
		delete this;
		return true;
	}

	if (config->isMappedTo("start", input) && input.value != 0)
	{		
		if (mMenu.size() && mOkCallback != nullptr)
		{
			auto path = mMenu.getSelected();

			if (mTypes == FileTypes::DIRECTORY && path.empty())
				onOk(mCurrentPath);				
			else if (!path.empty() && (mTypes == FileTypes::DIRECTORY || !Utils::FileSystem::isDirectory(path)))
				onOk(path);
		}

		return true;		
	}

	if (config->isMappedTo("x", input) && input.value && mOkCallback != nullptr)
	{
		onOk("");		
		return true;
	}
	
	if (config->isMappedTo("select", input))
	{
		navigateTo(Paths::getScreenShotPath());
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiFileBrowser::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	
	if (mOkCallback != nullptr)
		prompts.push_back(HelpPrompt("x", _("RESET")));

	prompts.push_back(HelpPrompt(BUTTON_BACK, _("CLOSE")));
	prompts.push_back(HelpPrompt("y", _("PARENT FOLDER")));
	prompts.push_back(HelpPrompt("select", _("SCREENSHOTS FOLDER")));
	prompts.push_back(HelpPrompt("start", _("SELECT")));

	return prompts;
}

void GuiFileBrowser::onOk(const std::string& path)
{
        if (Utils::FileSystem::isDirectory(mCurrentPath) && Settings::getInstance()->setString("LastFileBrowserFolder", mCurrentPath))
                Settings::getInstance()->saveFile();

        if (mOkCallback)
                mOkCallback(path);

        delete this;
}

//
// GuiImageFileBrowser implementation
//

GuiImageFileBrowser::GuiImageFileBrowser(Window* window, const std::string startPath, const std::string selectedFile,
        const std::function<void(const std::string&)>& okCallback, const std::string& title) :
        GuiComponent(window), mGrid(window)
{
        setTag("popup");

        mSelectedFile = Utils::FileSystem::getCanonicalPath(selectedFile);
        mOkCallback = okCallback;

        addChild(&mGrid);

        auto theme = ThemeData::getMenuTheme();
        float sh = (float)Math::min(Renderer::getScreenHeight(), Renderer::getScreenWidth());
        sh = (float)theme->TextSmall.font->getSize() / sh;

        std::string xml =
                "<theme defaultView=\"Tiles\">"
                "<formatVersion>7</formatVersion>"
                "<view name=\"grid\">"
                "<imagegrid name=\"gamegrid\">"
                "  <margin>0.01 0.02</margin>"
                "  <padding>0 0</padding>"
                "  <scrollDirection>vertical</scrollDirection>"
                "  <autoLayout>4 3</autoLayout>"
                "  <autoLayoutSelectedZoom>1</autoLayoutSelectedZoom>"
                "  <animateSelection>false</animateSelection>"
                "  <centerSelection>false</centerSelection>"
                "</imagegrid>"
                "<gridtile name=\"default\">"
                "  <backgroundColor>FFFFFF00</backgroundColor>"
                "  <padding>8 8</padding>"
                "  <imageColor>FFFFFFFF</imageColor>"
                "</gridtile>"
                "<gridtile name=\"selected\">"
                "  <backgroundColor>" + Utils::String::toHexString(theme->Text.selectorColor) + "</backgroundColor>"
                "</gridtile>"
                "<text name=\"gridtile\">"
                "  <color>" + Utils::String::toHexString(theme->Text.color) + "</color>"
                "  <backgroundColor>00000000</backgroundColor>"
                "  <fontPath>" + theme->TextSmall.font->getPath() + "</fontPath>"
                "  <fontSize>" + std::to_string(sh) + "</fontSize>"
                "  <alignment>center</alignment>"
                "  <singleLineScroll>false</singleLineScroll>"
                "  <size>1 0.30</size>"
                "</text>"
                "<text name=\"gridtile:selected\">"
                "  <color>" + Utils::String::toHexString(theme->Text.selectedColor) + "</color>"
                "</text>"
                "<image name=\"gridtile.image\">"
                "  <linearSmooth>true</linearSmooth>"
                "</image>"
                "</view>"
                "</theme>";

        mTheme = std::shared_ptr<ThemeData>(new ThemeData());
        std::map<std::string, std::string> emptyMap;
        mTheme->loadFile("imagebrowser", emptyMap, xml, false);
        mGrid.applyTheme(mTheme, "grid", "gamegrid", 0);
        mGrid.setCursorChangedCallback([&](const CursorState& /*state*/) { updateHelpPrompts(); });

        if (startPath.empty() || !Utils::FileSystem::isDirectory(startPath))
        {
                mCurrentPath = Settings::getInstance()->getString("LastFileBrowserFolder");
                if (mCurrentPath.empty() || !Utils::FileSystem::isDirectory(mCurrentPath))
                        mCurrentPath = Paths::getScreenShotPath();
                navigateTo(mCurrentPath);
        }
        else
                navigateTo(startPath);
}

void GuiImageFileBrowser::navigateTo(const std::string& path)
{
        mCurrentPath = path;
        mGrid.clear();
        mGrid.onSizeChanged();

        if (mCurrentPath != "\\" && mCurrentPath != "/" && !mCurrentPath.empty())
                mGrid.add("..", ":/folder.svg", Utils::FileSystem::getParent(mCurrentPath));

        auto files = Utils::FileSystem::getDirectoryFiles(mCurrentPath);
        files.sort([](const Utils::FileSystem::FileInfo& file1, const Utils::FileSystem::FileInfo& file2) {
                auto name1 = Utils::FileSystem::getFileName(file1.path);
                auto name2 = Utils::FileSystem::getFileName(file2.path);
                return Utils::String::compareIgnoreCase(name1, name2) < 0;
        });

        for (auto file : files)
        {
                if (file.hidden)
                        continue;

                if (file.directory)
                {
                        mGrid.add(Utils::FileSystem::getFileName(file.path), ":/folder.svg", file.path);
                        continue;
                }

                std::string ext = Utils::String::toLower(Utils::FileSystem::getExtension(file.path));
                if (ext == ".jpg" || ext == ".png" || ext == ".gif" || ext == ".svg")
                        mGrid.add(Utils::FileSystem::getFileName(file.path), file.path, file.path);
        }

        if (!mSelectedFile.empty())
                mGrid.setCursor(mSelectedFile);
}

void GuiImageFileBrowser::onSelected(const std::string& path)
{
        if (Utils::FileSystem::isDirectory(mCurrentPath) && Settings::getInstance()->setString("LastFileBrowserFolder", mCurrentPath))
                Settings::getInstance()->saveFile();

        if (mOkCallback)
                mOkCallback(path);

        delete this;
}

bool GuiImageFileBrowser::input(InputConfig* config, Input input)
{
        if (GuiComponent::input(config, input))
                return true;

        if (input.value != 0 && config->isMappedTo(BUTTON_BACK, input))
        {
                delete this;
                return true;
        }

        if (input.value != 0 && config->isMappedTo("y", input))
        {
                if (mCurrentPath != "\\" && mCurrentPath != "/" && !mCurrentPath.empty())
                        navigateTo(Utils::FileSystem::getParent(mCurrentPath));
                return true;
        }

        if (input.value != 0 && config->isMappedTo(BUTTON_OK, input))
        {
                if (mGrid.size())
                {
                        std::string path = mGrid.getSelected();
                        if (Utils::FileSystem::isDirectory(path))
                                navigateTo(path);
                        else
                                onSelected(path);
                }
                return true;
        }

        return false;
}

std::vector<HelpPrompt> GuiImageFileBrowser::getHelpPrompts()
{
        std::vector<HelpPrompt> prompts;
        prompts.push_back(HelpPrompt(BUTTON_OK, _("SELECT")));
        prompts.push_back(HelpPrompt("y", _("PARENT FOLDER")));
        prompts.push_back(HelpPrompt(BUTTON_BACK, _("CLOSE")));
        return prompts;
}