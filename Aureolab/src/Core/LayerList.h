#pragma once
#include "Layer.h"

#include <vector>

class LayerList {
public:
	void PushLayer(Layer* layer);
	void PopLayer();

	void InsertLayer(Layer* layer, unsigned int index);
	void RemoveLayer(Layer* layer);

	size_t Size();

	std::vector<Layer*>::const_iterator begin() const { return layers.begin(); }
	std::vector<Layer*>::const_iterator end() const { return layers.end(); }
protected:
	std::vector<Layer*> layers;
};