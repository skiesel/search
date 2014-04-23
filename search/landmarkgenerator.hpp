#pragma once

template <class D, class L> class LandmarkGenerator {
public:
	LandmarkGenerator() {}
	virtual L getLandmark(D&, typename D::State&, unsigned int) = 0;
	virtual unsigned int getLandmarkCount() const = 0;
};
