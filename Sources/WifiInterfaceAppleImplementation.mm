/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceLinuxAppleImplementation.mm
 *
 * This file contains Apple specific functions for managing WiFi adapters.
 *
 **/

#import "../Includes/WifiInterfaceAppleImplementation.h"
#import <CoreWLAN/CoreWLAN.h>

#include <cerrno>
#include <chrono>

#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>

#include "../Includes/Logger.h"
#include "../Includes/NetConversionFunctions.h"
#include "../Includes/WifiInterfaceApple.h"


WifiInterface::WifiInterface(std::string_view aAdapterName) :
    mImplementation([[WifiInterfaceAppleImplementation alloc] init]),
    mAdapterName(aAdapterName)
{
    
}

WifiInterface::~WifiInterface() = default;

bool WifiInterface::Connect(const IWifiInterface::WifiInformation& aConnection)
{
    return [mImplementation Connect : aConnection];
}

bool WifiInterface::LeaveIBSS()
{
    return [mImplementation LeaveIBSS];
}

uint64_t WifiInterface::GetAdapterMacAddress()
{
    uint64_t lReturn{0};

    ifaddrs* lInterfaceAddresses{nullptr};
    ifaddrs* lInterfaceAddress{nullptr};

    if (getifaddrs(&lInterfaceAddresses) == 0) {
        for (lInterfaceAddress = lInterfaceAddresses; (lInterfaceAddress != nullptr) && (lReturn == 0);
             lInterfaceAddress = lInterfaceAddress->ifa_next) {
            if ((lInterfaceAddress->ifa_name == mAdapterName) && (lInterfaceAddress->ifa_addr->sa_family == AF_LINK)) {
                unsigned char* lAddress = reinterpret_cast<unsigned char*>(
                    LLADDR(reinterpret_cast<sockaddr_dl*>(lInterfaceAddress->ifa_addr)));
                memcpy(&lReturn, lAddress, Net_8023_Constants::cSourceAddressLength);
            }
        }
        freeifaddrs(lInterfaceAddresses);
    } else {
        Logger::GetInstance().Log("Could not get network addresses", Logger::Level::ERROR);
    }
    return lReturn;
}

std::vector<IWifiInterface::WifiInformation>& WifiInterface::GetAdhocNetworks()
{
    return [mImplementation GetAdhocNetworks];
}


@implementation WifiInterfaceAppleImplementation

- (id) init
{
  self = [super init];

  if (self!=nil) {
    sharedWiFiClient = CWWiFiClient.sharedWiFiClient;
    wifiInterfaceName = @"";
    wifiInterface = nil;
    networks = nil;
  }

  return self;
}

- (bool) SetInterfaceWithName : (NSString*) wifiInterfaceName
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
    if (wifiInterface != nil) {
  Logger::GetInstance().Log("Connecting to: " + connection.ssid, Logger::Level::TRACE);

  if (connection.bssid.at(0) == 0 && connection.bssid.at(1) == 0) {
    // If no BSSID, create network (or join existing hopefully)
    // TODO: Figure out if this can actually join existing networks
    NSString* ssidNSString = [NSString stringWithCString:connection.ssid.c_str()
                                       encoding:[NSString defaultCStringEncoding]];
    
    NSData* ssidNSData = [ssidNSString dataUsingEncoding:NSUTF8StringEncoding];
      
    BOOL success = [wifiInterface startIBSSModeWithSSID : ssidNSData
                                                          security : kCWIBSSModeSecurityNone
                                                          channel : ConvertFrequencyToChannel(connection.frequency)
                                                          password : nil
                                                          error : nil];
      
    return success == YES;
      
  } else {
    // If there was a bssid, grab the right CWNetwork from our saved scanResults
    NSSet* scanResults = [wifiInterface cachedScanResults];
      
    // Make sure we can dump this BSSID as a string by making this into a uint64_t that IntToMac can pickup
    uint64_t bssidAsInt = 0;
    memcpy(&bssidAsInt, connection.bssid.data(), connection.bssid.size());

      
    NSString* targetBssidAsString = [NSString stringWithCString : IntToMac(bssidAsInt).c_str()
                                              encoding : [NSString defaultCStringEncoding]];

    // Make list of networks fitting the bssid we got.
    NSSet* filteredResults = [filteredResults objectsPassingTest: ^(CWNetwork* object ,BOOL *stop) {
      BOOL testResult = NO;
      NSString* bssid = object.bssid;
      if ([bssid isEqualToString : targetBssidAsString]) {
        testResult = YES;
      }

      return testResult;
    }];

    if ([filteredResults count] != 0) {
      NSArray* resultsAsArray = [filteredResults allObjects];
      CWNetwork* network = [resultsAsArray objectAtIndex : 0];

      // Start connecting
      BOOL success = [wifiInterface associateToNetwork: network
                                                        password: nil
                                                        error: nil];
    
      return success == YES;
    }
  }
    } else {
        Logger::GetInstance().Log("Wifi interface did not exist! can't continue", Logger::Level::ERROR);
    }

  return false;
}

- (bool) LeaveIBSS
{
  if (wifiInterface != nil)
  {
    [wifiInterface disassociate];

    return true;
  } else {
      Logger::GetInstance().Log("Wifi interface did not exist! can't continue", Logger::Level::ERROR);

  }
    return false;
}

- (std::vector<IWifiInterface::WifiInformation>&) GetAdhocNetworks
{
    if (wifiInterface != nil)
    {
    networks = [wifiInterface scanForNetworksWithSSID: nil
                              includeHidden : NO
                              error: nil];
        
        for(CWNetwork* network in networks) {
            if(network.ibss) {
                std::string bssid = std::string([network.bssid UTF8String]);
                uint64_t bssidAsInt = MacToInt(bssid);
                std::array<uint8_t, 6> bssidAsArray = {};
                memcpy(bssidAsArray.data(), &bssidAsInt, 6);
                
                IWifiInterface::WifiInformation information;
                information.frequency = ConvertChannelToFrequency(network.wlanChannel.channelNumber);
                information.bssid = bssidAsArray;
                information.isadhoc = true;
                information.isconnected = [wifiInterface bssid] == network.bssid;
                
                externalNetworks.push_back(information);
            }
        }
        
        
    } else {
        Logger::GetInstance().Log("Wifi interface did not exist! can't continue", Logger::Level::ERROR);
    }
    
    return externalNetworks;
}

@end
