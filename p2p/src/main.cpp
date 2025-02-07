#include <memory>
#include "signaling.h"

int main() {
    auto c = std::make_unique<WebsocketSignalingClient>("127.0.0.1", "8080", "/renderer");
    c->start();
    return 0;
}