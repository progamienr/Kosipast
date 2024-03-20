#include "Notifications.h"

// Myzarfin added this
void CNotifications::Draw()
{
	constexpr int x{ 1 };
	int y{ 1 };
	constexpr int size{ 20 };

	if (NotificationTexts.size() > (MAX_NOTIFY_SIZE + 1))
	{
		NotificationTexts.erase(NotificationTexts.begin());
	}

	for (size_t i{}; i < NotificationTexts.size(); ++i)
	{
		const auto notify = NotificationTexts[i];

		notify->Time -= I::GlobalVars->absoluteframetime;

		if (notify->Time <= 0.f)
		{
			NotificationTexts.erase(NotificationTexts.begin() + i);
		}
	}

	if (NotificationTexts.empty())
	{
		return;
	}

	const auto& font = g_Draw.GetFont(FONT_INDICATORS);
	for (size_t i{}; i < NotificationTexts.size(); ++i)
	{
		const auto notify = NotificationTexts[i];

		const float left = notify->Time;
		Color_t color = notify->Color;

		if (left < .5f)
		{
			float f = left;
			Math::Clamp(f, 0.f, .5f);

			f /= .5f;

			color.a = static_cast<int>(f * 255.f);

			if (i == 0 && f < 0.2f)
				y -= size * (1.f - f / 0.2f);
		}
		else
			color.a = 255;

		const size_t cSize = strlen(notify->Text.c_str()) + 1;
		const auto wc = new wchar_t[cSize];
		mbstowcs(wc, notify->Text.c_str(), cSize);

		int w, h;

		I::MatSystemSurface->GetTextSize(FONT_INDICATORS, wc, w, h);

		delete[] wc; // Memory leak

		g_Draw.Line(x, y, x, y + 19, { Vars::Menu::Theme::Accent.Value.r, Vars::Menu::Theme::Accent.Value.g, Vars::Menu::Theme::Accent.Value.b, color.a });
		g_Draw.GradientRect(x + 1, y, w / 3 + 8, 19,
			{ Vars::Menu::Theme::Background.Value.r, Vars::Menu::Theme::Background.Value.g, Vars::Menu::Theme::Background.Value.b, color.a },
			{ Vars::Menu::Theme::Background.Value.r, Vars::Menu::Theme::Background.Value.g, Vars::Menu::Theme::Background.Value.b, 1 },
			true);
		g_Draw.String(font, x + 6, y + 2,
			{ Vars::Menu::Theme::Active.Value.r, Vars::Menu::Theme::Active.Value.g, Vars::Menu::Theme::Active.Value.b, color.a },
			ALIGN_TOPLEFT, notify->Text.c_str());

		y += size;
	}
}