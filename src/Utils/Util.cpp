#include "Util.h"
#include "NMB.h"

void Util::ErrorPopup(std::string message) {
        LOGGER(LOGGER::ERR) << message;
        NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
        throw std::runtime_error(message.c_str());
}
