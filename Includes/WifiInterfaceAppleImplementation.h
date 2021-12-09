#import <Foundation/Foundation.h>

#include "../Includes/WifiInterfaceApple.h"

@class CWWiFiClient;
@class CWInterface;
@class CLLocationManager;

@interface WifiInterfaceAppleImplementation : NSObject {
  CWWiFiClient* sharedWiFiClient;
  NSString*     wifiInterfaceName;
  CWInterface*  wifiInterface;
  NSSet*      networks;
  std::vector<IWifiInterface::WifiInformation> externalNetworks;
  CLLocationManager* locationManager;
}

- (id) init;
- (bool) SetInterfaceWithName : (NSString*) wifiInterfaceName;
- (bool) Connect : (const struct IWifiInterface::WifiInformation&) connection;
- (bool) LeaveIBSS;
- (std::vector<IWifiInterface::WifiInformation>&) GetAdhocNetworks;

@end
