#pragma once
#include <imgui.h>
#include <string>
class GLFWwindow;

class ImApplication {
public:
  ImApplication(const std::string &title = "ImApplication Window");
  virtual ~ImApplication();

  int run();

protected:
  virtual void beforeLoop() {}
  virtual void paint();
  virtual void beforeQuit() {}

  inline ImGuiID dockSpaceID() const { return _dockSpaceId; }

private:
  std::string _windowTitle{};
  GLFWwindow *_glfwWindow{nullptr};
  int _displayW{}, _displayH{};
  ImVec4 _clearColor{0.45f, 0.55f, 0.60f, 1.00f};
  ImGuiID _dockSpaceId{};

  void init();
  void loop();
  void quit();

  static void onGlfwError(int error, const char *description) noexcept;
};
