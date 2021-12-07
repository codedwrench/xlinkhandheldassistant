/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceLinuxAppleImplementation.mm
 *
 * This file contains Apple specific functions for managing WiFi adapters.
 *
 **/

#import <CoreWLAN/CoreWLAN.h>

#include "Includes/IWifiInterface.h"
#include "Includes/Logger.h"


@interface WifiInterfaceAppleImplementation : NSObject {
  CWWiFiClient* sharedWiFiClient;
  NSString*     wifiInterfaceName;
  CWInterface*  wifiInterface;
  NSSet*        networks; 
  CWNetwork     connectTo;
}

- (id) init;
- (bool) setInterfaceWithName : (NSString*) wifiInterfaceName;
- (bool) Connect : (const struct IWifiInterface::WifiInformation&) connection;
- (bool) LeaveIBSS;

@end

@implementation WifiInterfaceAppleImplementation

- (id) init
{
  self = [super init];
    
  if (self!=nil) {
    sharedWiFiClient = CWWiFiClient.sharedWiFiClient;
    wifiInterfaceName = @"";
    wifiInterface = nil;
  }
    
  return self;
}

- (bool) setInterfaceWithName : (NSString*) wifiInterfaceName
{
  NSArray* interfaces = [CWWiFiClient interfaceNames];
  for (NSString* interfaceName in interfaces)
  {
    if ([wifiInterfaceName isEqualToString:interfaceName])
    {
      wifiInterface = [sharedWiFiClient interfaceWithName:wifiInterfaceName];
      return true;
    }
  }
  return false;
}

- (bool) Connect : (const struct IWifiInterface::WifiInformation&) connection;
{
    Logger::GetInstance().Log("Connecting to: " + aConnection.ssid, Logger::Level::TRACE);

    if (aConnection.bssid.at(0) == 0 && aConnection.bssid.at(1) == 0) {
       // If no BSSID, create network
       return [wifiInterface startIBSSModeWithSSID name:aConnection.ssid security:kCWIBSSModeSecurityNone channel:aConnection.channel] 
    }

}

- (bool) LeaveIBSS
{
  if (wifiInterface != nil)
  {
    [wifiInterface disassociate];
  }
}

@end
