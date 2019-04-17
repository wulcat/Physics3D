#include "panel.h"

#include "gui.h"

#include "../util/log.h"
#include "../engine/math/mathUtil.h"

Panel::Panel(double x, double y) : Container(Vec2(x, y)) {
	this->backgroundColor = GUI::defaultPanelBackgroundColor;
	this->padding = GUI::defaultPanelPadding;
	this->margin = GUI::defaultPanelMargin;
};

Panel::Panel(double x, double y, double width, double height) : Container(Vec2(x, y), Vec2(width, height)) {
	this->backgroundColor = GUI::defaultPanelBackgroundColor;
	this->padding = GUI::defaultPanelPadding;
	this->margin = GUI::defaultPanelMargin;
};

Vec2 Panel::resize() {
	// move in separate layout class
	if (layout == Layout::FLOW) {
		//Log::debug("flow");
		// Resulting width of the container
		double contentWidth = 0;
		// Resulting height of the container
		double contentHeight = 0;
		// Width of the current row of components
		double rowWidth = 0;
		// Height of the current row of components
		double rowHeight = 0;

		for (int i = 0; i < children.size(); i++) {
			auto child = children[i];
			Component* component = child.first;
			Align alignment = child.second;
			Vec2 componentSize = component->resize();
			
			// NO HEIGHT CHECK YET
			if (alignment == Align::FILL) {
				//Log::debug("fill %f, %f", componentSize.x, componentSize.y);
				double newRowWidth = rowWidth + componentSize.x;
				if (newRowWidth <= dimension.x || resizing) {
					// Set component position relative to parent
					component->position = position + Vec2(rowWidth, -contentHeight);
					
					// End of the current row, component keeps dimension
					rowWidth = newRowWidth;
					rowHeight = fmax(rowHeight, componentSize.y);

					// Resize the container so the component fits in
					contentWidth = fmax(contentWidth, rowWidth);

					// Next line
					contentHeight += rowHeight;

					// Reset row size
					rowWidth = 0;
					rowHeight = 0;
				} else {
					// NO CHECK IF COMPONENT WIDTH IS GREATER THAN CONTENTWIDTH
					// Component does not fit in the current row, advance to the next row
					contentHeight += rowHeight;

					// Set component position relative to parent
					component->position = position + Vec2(0, -contentHeight);

					// Advance position
					contentHeight += componentSize.y;

					// Reset row size
					rowWidth = 0;
					rowHeight = 0;
				}
			} else if (alignment == Align::RELATIVE) {
				//Log::debug("relative %f, %f", componentSize.x, componentSize.y);
				double newRowWidth = rowWidth + componentSize.x;
				if (newRowWidth <= dimension.x || resizing) {
					// Set component position relative to parent
					component->position = position + Vec2(rowWidth, -contentHeight);

					// Add component to current row, resize row height
					rowWidth = newRowWidth;
					rowHeight = fmax(rowHeight, componentSize.y);

					// Resize the container
					contentWidth = fmax(contentWidth, rowWidth);
				} else {
					// Component does not fit in the current row, advance to the next row
					contentHeight += rowHeight;

					// Set component position relative to parent
					component->position = position + Vec2(0, -contentHeight);

					// Set new row size
					rowWidth = componentSize.x;
					rowHeight = componentSize.y;
				}
			}
			//Log::debug("\tplaced %f, %f", component->position.x, component->position.y);
		}

		
		// Add height of last row
		contentHeight += rowHeight;
		
		if (resizing) {
			dimension = Vec2(contentWidth, contentHeight);
		}
	}

	return dimension;
}

void Panel::render() {
	if (visible) {
		resize();

		// padding and margin to be added
		GUI::defaultShader->update(backgroundColor);
		GUI::defaultQuad->resize(position, dimension);
		GUI::defaultQuad->render();

		renderChildren();
	}
}
