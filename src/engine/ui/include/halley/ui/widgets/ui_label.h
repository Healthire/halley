#pragma once
#include "../ui_widget.h"
#include "halley/core/graphics/text/text_renderer.h"
#include <climits>
#include "halley/text/i18n.h"

namespace Halley {
	class UILabel : public UIWidget {
	public:
		explicit UILabel(const String& id, TextRenderer style, const LocalisedString& text);

		void setText(const LocalisedString& text);
		void setColourOverride(const std::vector<ColourOverride>& overrides);

		void setMaxWidth(float maxWidth);
		void setMaxHeight(float maxHeight);
		float getMaxWidth() const;
		float getMaxHeight() const;

		void setAlignment(float alignment);

		void setTextRenderer(TextRenderer renderer);
		TextRenderer& getTextRenderer();
		const TextRenderer& getTextRenderer() const;

		void setColour(Colour4f colour);
		Colour4f getColour() const;

		void setSelectable(Colour4f normalColour, Colour4f selColour);

		void draw(UIPainter& painter) const override;
		void update(Time t, bool moved) override;

	private:
		TextRenderer renderer;
		LocalisedString text;

		Vector2f textExtents;
		float maxWidth = std::numeric_limits<float>::infinity();
		float maxHeight = std::numeric_limits<float>::infinity();
		bool needsClip = false;

		void updateMinSize();
		void updateText();
	};
}
