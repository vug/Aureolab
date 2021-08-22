#include "LayerList.h"

#include <cassert>

void LayerList::PushLayer(Layer* layer) {
	layer->OnAttach();
	layers.push_back(layer);
}

void LayerList::PopLayer() {
	assert(layers.size() > 0); // at least needs one layer to pop it
	auto layer = layers.back();
	layer->OnDetach();
	layers.pop_back();
}

void LayerList::InsertLayer(Layer* layer, unsigned int index) {
	assert(index <= Size()); // cannot insert after 
	layer->OnAttach();
	layers.insert(layers.begin() + index, layer);
}

void LayerList::RemoveLayer(Layer* layer) {
	auto it = std::find(layers.begin(), layers.end(), layer);
	assert(it != layers.end()); // cannot remove a layer that's not in LayerList
	layers.erase(it);
}

size_t LayerList::Size() {
	return layers.size();
}
