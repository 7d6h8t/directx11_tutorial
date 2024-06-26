#include "pch.h"
#include "framework/system_class.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  SystemClass* system = new SystemClass{};
  if (system == nullptr) return -1;

  if (system->Initialize()) system->Run();

  system->Shutdown();
  delete system;
  system = nullptr;

  return 0;
}
