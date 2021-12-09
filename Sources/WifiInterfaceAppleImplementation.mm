/* Copyright (c) 2021 [Rick de Bondt] - WifiInterfaceAppleImplementation.mm
 *
 * This file contains Apple specific functions for managing WiFi adapters.
 *
 **/

#import "../Includes/WifiInterfaceAppleImplementation.h"
#import <CoreWLAN/CoreWLAN.h>
#import <CoreLocation/CoreLocation.h>

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
  [mImplementation SetInterfaceWithName : [NSString stringWithCString : aAdapterName.data()
                                                              encoding:[NSString defaultCStringEncoding]]];
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
    locationManager = [[CLLocationManager alloc] init];
     
    // TODO: This is broken on CLI for some stupid reason
    //if (@available(macOS 10.15, *)) {
    //    // Needed so that BSSIDs can be obtained
    //    [locationManager requestAlwaysAuthorization];
    //    [locationManager startUpdatingLocation];
    //}
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
  Logger::GetInstance().Log("Connecting to: " + connection.ssid + " Channel: " + std::to_string(ConvertFrequencyToChannel(connection.frequency)), Logger::Level::TRACE);

  // Grab the right CWNetwork from our saved scanResults
      
    NSString* targetSsidAsString = [NSString stringWithCString : connection.ssid.c_str()
                                              encoding : [NSString defaultCStringEncoding]];

    // Make list of networks fitting the ssid we got.
    NSSet* filteredResults = [networks objectsPassingTest: ^(CWNetwork* object ,BOOL *stop) {
      BOOL testResult = NO;
      if ([object.ssid isEqualToString : targetSsidAsString]) {
        testResult = YES;
      }

      return testResult;
    }];

    if ([filteredResults count] != 0) {
        Logger::GetInstance().Log(std::to_string([filteredResults count]) + " networks matched the filter", Logger::Level::TRACE);

      NSArray* resultsAsArray = [filteredResults allObjects];
      CWNetwork* network = [resultsAsArray objectAtIndex : 0];

      // Start connecting
      BOOL success = [wifiInterface associateToNetwork: network
                                                        password: nil
                                                        error: nil];
    
      return success == YES;
    } else {
        // If no SSID, create network (or join existing hopefully)
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
      Logger::GetInstance().Log("Network left", Logger::Level::DEBUG);


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
        NSError *error = nil;

        networks = [wifiInterface scanForNetworksWithSSID: nil
                                         error: &error];
        if (error)
        {
            Logger::GetInstance().Log("Scanning gave error: " + std::to_string(error.code), Logger::Level::ERROR);
            return externalNetworks;
        }
        
        externalNetworks.clear();
        
        if([networks count] != 0)
        {
            NSArray* networksArray = [networks allObjects];
            for(CWNetwork* network in networksArray) {
            if([network ibss]) {
                std::string ssid = [[network ssid] UTF8String];
                std::string bssid = ([network bssid] == nil) ? "00:00:00:00:00:00" : [[network bssid] UTF8String];
                
                Logger::GetInstance().Log(std::string("Found: ") + ssid + " with bssid: " + bssid, Logger::Level::DEBUG);

                uint64_t bssidAsInt = MacToInt(bssid);
                std::array<uint8_t, 6> bssidAsArray = {};
                memcpy(bssidAsArray.data(), &bssidAsInt, 6);
                
                IWifiInterface::WifiInformation information;
                information.ssid = ssid;
                information.frequency = ConvertChannelToFrequency(network.wlanChannel.channelNumber);
                information.bssid = bssidAsArray;
                information.isadhoc = true;
                information.isconnected = false;
                
                externalNetworks.push_back(information);
            
            }
                
            }
        }
        
        
    } else {
        Logger::GetInstance().Log("Wifi interface did not exist! can't continue", Logger::Level::ERROR);
    }
    
    return externalNetworks;
}

@end
