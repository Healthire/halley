#pragma once
#include "ui_clickable.h"

namespace Halley {
	class UIMenuButtonGroup;

	class UIMenuButton : public UIClickable {
	public:
		using OnGroupStateCallback = std::function<void(State)>;

		UIMenuButton(std::shared_ptr<UIMenuButtonGroup> group, String id, Vector2f minSize = {}, Maybe<UISizer> sizer = {}, Vector4f innerBorder = {});

		void onClicked(Vector2f mousePos) override;
		void onOptionChosen();
		void setGroupFocused(bool focused);
		bool isGroupFocused() const;

		void setOnGroupStateCallback(OnGroupStateCallback callback);

	protected:
		void update(Time t, bool moved) override;
		void doSetState(State state) override final;
		virtual void onGroupState(State state);

	private:
		std::shared_ptr<UIMenuButtonGroup> group;
		OnGroupStateCallback onGroupStateCallback;
		bool groupFocused = false;
	};

	class UIMenuButtonGroup {
	public:
		void addButton(std::shared_ptr<UIMenuButton> button, const String& up = "", const String& down = "", const String& left = "", const String& right = "");
		void setCancelId(const String& id);

		void onInput(const UIInputResults& input, Time time);
		bool setFocus(UIMenuButton& uiMenuButton);
		bool setFocus(const String& id);
		std::shared_ptr<UIMenuButton> getCurrentFocus() const;
		size_t size() const;
		
		void setEnabled(bool enabled);

	private:
		struct ButtonEntry {
			std::weak_ptr<UIMenuButton> button;
			String id;
			String up;
			String down;
			String left;
			String right;
		};

		std::vector<ButtonEntry> buttons;
		String curFocus;
		String cancelId;
		bool enabled = true;

		const ButtonEntry& getCurFocusEntry() const;
		ButtonEntry& getCurFocusEntry();
	};

	class UIMenuButtonControlWidget : public UIWidget {
	public:
		UIMenuButtonControlWidget(std::shared_ptr<UIMenuButtonGroup> group);

	protected:
		void onInput(const UIInputResults& input, Time time) override;

	private:
		std::shared_ptr<UIMenuButtonGroup> group;
	};

	class UIMenuButtonGroupHighlight {
	public:
		using FocusChangedCallback = std::function<void(const String&, bool)>; // Id of focus, is this the initial change?

		explicit UIMenuButtonGroupHighlight(std::shared_ptr<UIMenuButtonGroup> group);

		void setFocusChangedCallback(FocusChangedCallback callback);

		void update(Time t);
		Rect4f getCurRect() const;
		Time getElapsedTime() const;

	private:
		std::shared_ptr<UIMenuButtonGroup> group;

		Time transitionTime = -1;
		Time elapsedTime = 0;
	
		String lastFocus;
		Rect4f prevRect;
		Rect4f targetRect;
		Rect4f curRect;

		FocusChangedCallback focusChangedCallback;

		void onFocusChanged(const String& id, const String& previous);
	};
}
