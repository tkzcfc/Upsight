//
//  Created by Robert Segal on 2014-04-01.
//  Copyright (c) 2016 Get Set Games Inc. All rights reserved.
//

#pragma once

#include "UpsightFunctions.h"
#include "UpsightComponent.generated.h"

UCLASS(ClassGroup=Advertising, HideCategories=(Activation, "Components|Activation", Collision), meta=(BlueprintSpawnableComponent))
class UUpsightComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	void OnRegister() override;
	void OnUnregister() override;
	
    // Billboard callbacks
    //
    DECLARE_MULTICAST_DELEGATE_OneParam(FUpsightBillboardDidDismissDelegate,  const FString&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FUpsightBillboardWillDismissDelegate, const FString&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FUpsightBillboardDidAppearDelegate,   const FString&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FUpsightBillboardWillAppearDelegate,  const FString&);
    // Billboard delegates
    //
    static FUpsightBillboardDidDismissDelegate  BillboardDidDismissDelegate;
    static FUpsightBillboardWillDismissDelegate BillboardWillDismissDelegate;
    static FUpsightBillboardDidAppearDelegate   BillboardDidAppearDelegate;
    static FUpsightBillboardWillAppearDelegate  BillboardWillAppearDelegate;
    // Billboard delegates
    //
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FUpsightBillboardDidAppearDynDelegate,   const FString&, Scope);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FUpsightBillboardWillAppearDynDelegate,  const FString&, Scope);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FUpsightBillboardDidDismissDynDelegate,  const FString&, Scope);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FUpsightBillboardWillDismissDynDelegate, const FString&, Scope);
    UPROPERTY(BlueprintAssignable)
    FUpsightBillboardDidAppearDynDelegate BillboardDidAppear;
    
    UPROPERTY(BlueprintAssignable)
    FUpsightBillboardWillAppearDynDelegate BillboardWillAppear;
    
    UPROPERTY(BlueprintAssignable)
    FUpsightBillboardDidDismissDynDelegate BillboardDidDismiss;
    
    UPROPERTY(BlueprintAssignable)
    FUpsightBillboardWillDismissDynDelegate BillboardWillDismiss;
    

protected:
    
    void BillboardDidAppearDelegate_Handler  (const FString& scope);
    void BillboardWillAppearDelegate_Handler (const FString& scope);
    void BillboardDidDismissDelegate_Handler (const FString& scope);
    void BillboardWillDismissDelegate_Handler(const FString& scope);
    
private:
    void ApplicationRegisteredForRemoteNotifications_Handler(TArray<uint8> Token);
    void ApplicationRegisteredForUserNotificationsDelegate_Handler(int32 inInt);
    void ApplicationFailedToRegisterForRemoteNotifications_Handler(FString RegisterError);
    void ApplicationReceivedRemoteNotification_Handler(FString Json);

};
