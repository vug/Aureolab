#pragma once

class IResizable {
public:
	virtual void OnResize(int width, int height) = 0;
};