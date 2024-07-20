#include "Util.h"

#if defined(_WIN64)
#include "NMB.h"
#endif

void Util::ErrorPopup(std::string message) {
        LOGGER(LOGGER::ERR) << message;
#if defined(_WIN64)
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
#endif
        throw std::runtime_error(message.c_str());
}
