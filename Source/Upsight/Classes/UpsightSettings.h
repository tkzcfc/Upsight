//
//  Created by Robert Segal on 2014-04-01.
//  Copyright (c) 2016 Get Set Games Inc. All rights reserved.
//

#pragma once

#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#include "UpsightSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class UUpsightSettings : public UObject
{
	GENERATED_BODY()
	
public:
	UUpsightSettings(const FObjectInitializer& ObjectInitializer);
	
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Application Token"))
    FString ApplicationToken;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Public Key"))
    FString PublicKey;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Include Analytics"))
    bool IncludeAnalytics;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Include Managed Variables"))
    bool IncludeManagedVariables;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Include Marketing"))
    bool IncludeMarketing;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Include Advertising ID"))
    bool IncludeAdvertisingId;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Include Push Notifications"))
    bool IncludePushNotifications;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Push Notification Sender ID"))
    FString PushNotificationSenderID;
    
    UPROPERTY(Config, EditAnywhere, Category=General, meta=(DisplayName="Enable Logging"))
    bool EnableLogging;
};
