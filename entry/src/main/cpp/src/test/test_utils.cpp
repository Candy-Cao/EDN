#include "edn_socket_utils.h"
#include "edn_log.h"
#include "edn_define.h"
#include "edn_utils.h"

using namespace edn;
using namespace std;

int main() {
    // Test GetDefaultIpV4
    string ip = GetDefaultIpV4();
    EDN_LOG_INFO("Default IPv4: %s", ip.c_str());

    // Test GetDefaultIpV6
    string ipv6 = GetDefaultIpV6();
    EDN_LOG_INFO("Default IPv6: %s", ipv6.c_str());

    return 0;
}