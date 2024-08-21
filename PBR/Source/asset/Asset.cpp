#include "Asset.h"
#include "../utils/Log.h"
namespace asset {
	Asset::Asset():mId(0){
		LOG_TRACK;
	}

	Asset::Asset(Asset&& other) noexcept{
		LOG_TRACK;
		mId = std::exchange(other.mId, 0);
	}

	Asset& Asset::operator=(Asset&& other) noexcept{
		LOG_TRACK;
		if (this != &other) {
			this->mId = std::exchange(other.mId, 0);
		}
		return *this;
	}
}
