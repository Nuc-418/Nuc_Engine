// WindowChrome: replaces the OS title bar with a borderless window whose caption
// is drawn by the editor (see Editor::DrawMenuBar). Native move, resize, Aero
// Snap and maximize are preserved through the non-client hit-test. Windows-only;
// every entry point is a no-op on other platforms.

#pragma once

struct GLFWwindow;

namespace WindowChrome
{
	// Strip the native frame and install the non-client hit-test. Call once,
	// after the ImGui GLFW backend has installed its own window procedure so
	// this one wraps (and forwards to) it.
	void Install(GLFWwindow* window);

	// Publish the caption layout each frame so the native hit-test knows which
	// parts of the top bar drag the window and which stay clickable. Coordinates
	// are in window/client pixels from the top-left:
	//   height       - title bar height
	//   menuRightEdge - right edge of the left-aligned menu items (clickable)
	//   buttonsLeft   - left edge of the window control buttons (clickable)
	// The strip between the two is the draggable caption.
	void SetTitleBar(float height, float menuRightEdge, float buttonsLeft);
}
