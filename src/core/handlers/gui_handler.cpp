#include "settings.h"
#include "core_utils.h"
#include "gui_utils.h"
#include "gui_handler.h"

using namespace VisoGui::Core::Utils;
using namespace VisoGui::Core::Models;
using namespace VisoGui::Gui::Models;

using namespace VisoGui::Core::Handlers;

GuiHandler::GuiHandler(const State &state)
	: state(state),
	  infoWindow(),
	  videoWindow()
{
}

void GuiHandler::Initialize(HINSTANCE hInstance)
{
	// Init info window
	this->infoWindow.Initialize(
		hInstance,
		"win-info",
		"LibViso2 GUI (Stereo)",
		WindowLocation(0, 0, 1070, 650),
		std::make_unique<PaintDrawFunction>(
			[this](Gdiplus::Graphics &graphics)
			{ this->OnInfoPaint(graphics); }));

	// Init video preview window
	this->videoWindow.Initialize(
		hInstance,
		"win-video",
		"Video processing",
		WindowLocation(),
		std::make_unique<PaintTargetFunction>(
			[this]()
			{ return this->state.VO.img_file_name; }));
}

void GuiHandler::OnInfoPaint(Gdiplus::Graphics &graphics)
{
	this->DrawVOParams(graphics);
	this->DrawVOStats(graphics);
	this->DrawVOGraph(graphics);
}

void GuiHandler::DrawVOParams(Gdiplus::Graphics &graphics)
{
	GuiUtils::DrawString(graphics,
						 "Calibration Parameters:",
						 Settings::Gui::xInfoText,
						 280);

	auto params = &this->state.VO.params;

	// Draw VO Calibration parameters

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("focus(pixels) = %.2f", params->calib.f),
						 Settings::Gui::xInfoText,
						 295);

	GuiUtils::DrawString(graphics,
						 "(f[pixels] = img_width[pixels]*f[mm]/CCD_width[mm])",
						 Settings::Gui::xInfoText,
						 310);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("principal point (u-coordinate) px = %.2f", params->calib.cu),
						 Settings::Gui::xInfoText,
						 325);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("principal point (v-coordinate) px = %.2f", params->calib.cv),
						 Settings::Gui::xInfoText,
						 340);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("height/baseline (meters) = %.4f", params->base),
						 Settings::Gui::xInfoText,
						 355);
}

void GuiHandler::DrawVOStats(Gdiplus::Graphics &graphics)
{
	// App static information
	GuiUtils::DrawString(graphics,
						 "libViso2 - GUI",
						 Settings::Gui::xInfoText,
						 10);

	GuiUtils::DrawString(graphics,
						 "Video Processing for Visual Odometry.",
						 Settings::Gui::xInfoText,
						 25);

	int frame = this->state.VO.frame;
	double x = this->state.VO.x;
	double y = this->state.VO.y;
	double z = this->state.VO.z;
	double num_matches = this->state.VO.num_matches;
	double num_inliers = this->state.VO.num_inliers;
	auto img_file_name = this->state.VO.img_file_name;

	// Draw processing information
	GuiUtils::DrawString(graphics,
						 "Current Elaboration:",
						 Settings::Gui::xInfoText,
						 60);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("Current Frame: %i", frame),
						 Settings::Gui::xInfoText,
						 75);

	// Draw matches/inliers
	if (num_matches > 0)
	{
		GuiUtils::DrawString(graphics,
							 CoreUtils::Format("Matches: %f", num_matches),
							 Settings::Gui::xInfoText,
							 90);

		GuiUtils::DrawString(graphics,
							 CoreUtils::Format("Inliers (Overall %%): %f %%", (100.0 * num_inliers / num_matches)),
							 Settings::Gui::xInfoText,
							 105);
	}

	// Draw pose coordinates
	GuiUtils::DrawString(graphics,
						 "Current Pose:",
						 Settings::Gui::xInfoText,
						 140);

	GuiUtils::DrawString(graphics,
						 "(only X,Y,Z extracted from Matrix)",
						 Settings::Gui::xInfoText,
						 155);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("X Pose: %f", x),
						 Settings::Gui::xInfoText,
						 170);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("Y Pose: %f", y),
						 Settings::Gui::xInfoText,
						 185);

	GuiUtils::DrawString(graphics,
						 CoreUtils::Format("Z Pose: %f", z),
						 Settings::Gui::xInfoText,
						 200);

	// Draw filename
	GuiUtils::DrawString(graphics,
						 "Processing File:",
						 Settings::Gui::xInfoText,
						 235);

	if (this->state.VO.img_file_name != "")
	{
		GuiUtils::DrawString(graphics,
							 (char *)img_file_name.c_str(),
							 Settings::Gui::xInfoText,
							 250);
	}
}

void GuiHandler::DrawVOGraph(Gdiplus::Graphics &graphics)
{
	// Draw X-Z Graph box
	GuiUtils::DrawRectangle(
		graphics,
		Settings::Gui::xPadding,
		10,
		410,
		380);

	// X Axis
	GuiUtils::DrawLine(
		graphics,
		Settings::Gui::xPadding + 15,
		Settings::Gui::zOffset,
		Settings::Gui::xPadding + 395,
		Settings::Gui::zOffset);

	GuiUtils::DrawString(graphics,
						 "X",
						 Settings::Gui::xOffset + 178,
						 Settings::Gui::zOffset + 6);

	// Z Axis
	GuiUtils::DrawLine(
		graphics,
		Settings::Gui::xOffset,
		20,
		Settings::Gui::xOffset,
		380);

	GuiUtils::DrawString(graphics,
						 "Z",
						 Settings::Gui::xOffset - 15,
						 Settings::Gui::zOffset - 218);

	// Draw Y-Frames Graph box
	GuiUtils::DrawRectangle(
		graphics,
		Settings::Gui::xPadding,
		Settings::Gui::yOffset - 90,
		1010,
		180);

	// Frames Axis
	GuiUtils::DrawLine(
		graphics,
		Settings::Gui::fOffset - 20,
		Settings::Gui::yOffset,
		Settings::Gui::xPadding + 993,
		Settings::Gui::yOffset);

	GuiUtils::DrawString(graphics,
						 "f (frame)",
						 Settings::Gui::xPadding + 941,
						 Settings::Gui::yOffset + 6);

	// Y Axis
	GuiUtils::DrawLine(
		graphics,
		Settings::Gui::fOffset,
		Settings::Gui::yOffset - 80,
		Settings::Gui::fOffset,
		Settings::Gui::yOffset + 80);

	GuiUtils::DrawString(graphics,
						 "Y",
						 Settings::Gui::fOffset - 15,
						 Settings::Gui::yOffset - 79);

	// Draws X-Z Graph points (draw line to represent a point, thicker graphic)
	auto xzGraph = this->state.VO.xzGraph;
	for (auto &point : xzGraph)
	{
		int drawX = Settings::Gui::xOffset + point.first;
		int drawZ = Settings::Gui::zOffset - point.second;

		GuiUtils::DrawLine(
			graphics,
			drawX,
			drawZ,
			drawX + Settings::Gui::pointThickness,
			drawZ);
	}

	// Draw Y/frame Graph points (draw line to represent a point, thicker graphic)
	auto yfGraph = this->state.VO.yfGraph;
	for (auto &point : yfGraph)
	{
		int drawY = Settings::Gui::yOffset - point.first;
		int drawFrame = Settings::Gui::fOffset + point.second;

		GuiUtils::DrawLine(
			graphics,
			drawFrame,
			drawY,
			drawFrame,
			drawY + Settings::Gui::pointThickness);
	}
}
