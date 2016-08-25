#include "prec.h"
#include "test_stage.h"

using namespace Halley;

void initOpenGLPlugin(IPluginRegistry &registry);

namespace Stages {
	enum Type
	{
		Test
	};
}

class NetworkTestGame final : public Game
{
public:
	int initPlugins(IPluginRegistry &registry) override
	{
		initOpenGLPlugin(registry);
		return HalleyAPIFlags::Video | HalleyAPIFlags::Audio | HalleyAPIFlags::Input;
	}

	void initResourceLocator(String dataPath, ResourceLocator& locator) override
	{
		locator.addFileSystem(dataPath);
	}

	std::unique_ptr<Stage> makeStage(StageID id) override
	{
		switch (id) {
		case Stages::Test:
			return std::make_unique<TestStage>();
		default:
			return std::unique_ptr<Stage>();
		}
	}

	StageID getInitialStage() const override
	{
		return Stages::Test;
	}

	String getName() const override
	{
		return "Network Test";
	}

	String getDataPath() const override
	{
		return "halley/network-test";
	}

	bool isDevBuild() const override
	{
		return true;
	}

	void init(HalleyAPI* api, const Environment& environment, const Vector<String>& args) override
	{
		api->video->setWindow(Window(WindowType::Window, api->video->getCenteredWindow(Vector2i(1280, 720), 0), Vector2i(1280, 720), getName()), false);
	}
};

HalleyGame(NetworkTestGame);