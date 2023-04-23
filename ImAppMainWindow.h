#pragma once
#include <imgui.h>
#include <string>
class GLFWwindow;

namespace ImApp {
class MainWindow {
public:
  MainWindow(const std::string &title = "Main Window");
  virtual ~MainWindow();

  int run();

protected:
  virtual void beforeLoop() {}
  virtual void firstPaint() {}
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
  void frame();
  void render();
  void quit();

  static void onGlfwError(int error, const char *description) noexcept;
};
} // namespace ImApp
