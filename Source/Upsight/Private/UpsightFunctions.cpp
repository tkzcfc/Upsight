//
//  Created by Robert Segal on 2014-04-01.
//  Copyright (c) 2016 Get Set Games Inc. All rights reserved.
//

#include "UpsightFunctions.h"
#include "UpsightPrivatePCH.h"

#if PLATFORM_IOS
#import <UpsightKit/UpsightKit.h>
#elif PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "AndroidApplication.h"
#endif

#if PLATFORM_IOS

@interface UUpsightFunctionsDelegate : NSObject <USBillboardDelegate>
-(UIViewController *)presentingViewControllerForBillboard:(id<USBillboard>)aBillboard;
@end

static UUpsightFunctionsDelegate* ufd = nil;

@implementation UUpsightFunctionsDelegate

+(void)load
{
    if (!ufd)
    {
        ufd = [[UUpsightFunctionsDelegate alloc] init];
    }
}

-(void)billboardWillAppear:(id<USBillboard>)aBillboard
{
    UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - billboardWillAppear: - scope '%s'"), *FString(aBillboard.scope));
    
    UUpsightComponent::BillboardWillAppearDelegate.Broadcast(FString(aBillboard.scope));
}

-(void)billboardDidAppear:(id<USBillboard>)aBillboard
{
     UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - billboardDidAppear: - scope '%s'"), *FString(aBillboard.scope));
    
     UUpsightComponent::BillboardDidAppearDelegate.Broadcast(FString(aBillboard.scope));
}

-(void)billboardWillDismiss:(id<USBillboard>)aBillboard
{
    UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - billboardWillDismiss: - scope '%s'"), *FString(aBillboard.scope));
    
    UUpsightComponent::BillboardWillDismissDelegate.Broadcast(FString(aBillboard.scope));
}

-(void)billboardDidDismiss:(id<USBillboard>)aBillboard
{
    UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - billboardDidDismiss: - scope '%s'"), *FString(aBillboard.scope));
    
    UUpsightComponent::BillboardDidDismissDelegate.Broadcast(FString(aBillboard.scope));
    
    aBillboard.delegate = nil;
}

-(UIViewController *)presentingViewControllerForBillboard:(id<USBillboard>)aBillboard
{
    UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - presentingViewControllerForBillboard - presenting billboard for scope '%s'"), *FString(aBillboard.scope));
    
    UIViewController *topController = [UIApplication sharedApplication].keyWindow.rootViewController;
    
    while (topController.presentedViewController)
    {
        topController = topController.presentedViewController;
    }
    
    if (!topController)
    {
        UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - presentingViewControllerForBillboard - unable to find billboard for scope '%s'"), *FString(aBillboard.scope));
    }
    
    return topController;
}

-(void)billboard:(id<USBillboard>)aBillboard didReceiveReward:(id<USReward>)aReward
{
    UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - billboard:didReceiveReward: - scope:%s, reward:%s, quantity:%d, signatureData:%s"), *FString(aBillboard.scope), *FString(aReward.productIdentifier), aReward.quantity, *FString([NSString stringWithCString:(const char *)[aReward.signatureData bytes] encoding:NSUTF8StringEncoding]));
    
    TArray<UUpsightReward *> Rewards;
    
    UUpsightReward *r = NewObject<UUpsightReward>();
    
    r->Name          = FString(aReward.productIdentifier);
    r->Quantity      = aReward.quantity;
    r->SignatureData = FString([NSString stringWithCString:(const char *)[aReward.signatureData bytes] encoding:NSUTF8StringEncoding]);
    
    Rewards.Add(r);
    
    UUpsightComponent::DidReceieveRewardDelegate.Broadcast(Rewards);
}

-(void)billboard:(id<USBillboard>)aBillboard didReceivePurchase:(id<USPurchase>)aPurchase
{
    UE_LOG(LogUpsight, Log, TEXT("UpsightFunctions - billboard:didReceivePurchase: - scope:%s, purchase:%s, quantity:%d"), *FString(aBillboard.scope), *FString(aPurchase.productIdentifier), aPurchase.quantity);
    
    TArray<UUpsightVirtualGoodPromotionPurchase *> Purchases;
    
    UUpsightVirtualGoodPromotionPurchase *p = NewObject<UUpsightVirtualGoodPromotionPurchase>();
    
    p->Name     = FString(aPurchase.productIdentifier);
    p->Quantity = aPurchase.quantity;

    Purchases.Add(p);
    
    UUpsightComponent::DidReceieveVirtualGoodPromotionPurchaseDelegate.Broadcast(Purchases);
}

@end

#endif


bool ValidateValues(TArray<FString> &Keys, TArray<FString> &Values)
{
    if (Keys.Num() != Values.Num())
    {
        return false;
    }
    
    return true;
}

#if PLATFORM_IOS
NSDictionary* CreateNSDictionary(TArray<FString> &Keys, TArray<FString> &Values)
{
    const int32 kNumKeys = Keys.Num();
    
    NSMutableDictionary* d = [NSMutableDictionary dictionaryWithCapacity:kNumKeys];
    
    for (uint32 i = 0; i < kNumKeys; i++)
    {
        FString &k = Keys[i];
        FString &v = Values[i];
        
        d[k.GetNSString()] = v.GetNSString();
    }
    
    return d;
}
#endif

#if PLATFORM_ANDROID
void CreateJavaKeyValueArrays(JNIEnv *Env, jobjectArray &jKeysArray, jobjectArray &jValuesArray, TArray<FString> keys, TArray<FString> values)
{
    for (uint32 Param = 0; Param < keys.Num(); Param++)
    {
        jstring StringValue = Env->NewStringUTF(TCHAR_TO_UTF8(*keys[Param]));
        
        Env->SetObjectArrayElement(jKeysArray, Param, StringValue);
        Env->DeleteLocalRef(StringValue);
        
        StringValue = Env->NewStringUTF(TCHAR_TO_UTF8(*values[Param]));
        
        Env->SetObjectArrayElement(jValuesArray, Param, StringValue);
        Env->DeleteLocalRef(StringValue);
    }
}
#endif

void UUpsightFunctions::UpsightRecordAnalyticsEventWithName(FString eventName, TArray<FString> eventKeys, TArray<FString> eventValues)
{
    if ( ValidateValues(eventKeys, eventValues) )
    {
#if PLATFORM_IOS
        NSDictionary *p = CreateNSDictionary(eventKeys, eventValues);
    
        [Upsight recordAnalyticsEventWithName:eventName.GetNSString() properties: p];
    
#elif PLATFORM_ANDROID
        UE_LOG(LogUpsight, Log, TEXT("UUpsightFunctions::UpsightRecordAnalyticsEventWithName - event: %s"), *eventName);
        
        if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
        {
            static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                               FJavaWrapper::GameActivityClassID,
                                                               "AndroidThunkJava_UpsightRecordAnalyticsEventWithName",
                                                               "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
                                                               false);
            
            jobjectArray jKeys   = (jobjectArray)Env->NewObjectArray(eventKeys.Num(),   FJavaWrapper::JavaStringClass, NULL);
            jobjectArray jValues = (jobjectArray)Env->NewObjectArray(eventValues.Num(), FJavaWrapper::JavaStringClass, NULL);
            
            CreateJavaKeyValueArrays(Env, jKeys, jValues, eventKeys, eventValues);
            
            jstring jEventName = Env->NewStringUTF(TCHAR_TO_UTF8(*eventName));
            
            FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jEventName, jKeys, jValues);
            
            Env->DeleteLocalRef(jKeys);
            Env->DeleteLocalRef(jValues);
            Env->DeleteLocalRef(jEventName);
        }
#endif
    }
    else
    {
        UE_LOG(LogUpsight, Log, TEXT("keys and/or value arguments are empty or nil"));
    }
}

void UUpsightFunctions::UpsightRecordMilestoneEventForScope(FString scope,
                                                            TArray<FString> eventKeys,
                                                            TArray<FString> eventValues,
                                                            bool withCallbackSetup)
{
    if ( ValidateValues(eventKeys, eventValues) )
    {
#if PLATFORM_IOS
        NSDictionary *p = CreateNSDictionary(eventKeys, eventValues);

        dispatch_async(dispatch_get_main_queue(), ^{
            if (withCallbackSetup)
            {
                id<USBillboard> b = [Upsight billboardForScope:scope.GetNSString()];
                
                b.delegate = b.contentReady ? ufd : nil;
            }
            
            [Upsight recordMilestoneEventForScope:scope.GetNSString() properties:p];
        });
    
#elif PLATFORM_ANDROID
        UE_LOG(LogUpsight, Log, TEXT("UUpsightFunctions::UpsightRecordMilestoneEventForScope - scope: %s"), *scope);
        
        if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
        {
            static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                               FJavaWrapper::GameActivityClassID,
                                                               "AndroidThunkJava_UpsightRecordMilestoneEventForScope",
                                                               "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Z)V",
                                                               false);

            jobjectArray jKeys   = (jobjectArray)Env->NewObjectArray(eventKeys.Num(),   FJavaWrapper::JavaStringClass, NULL);
            jobjectArray jValues = (jobjectArray)Env->NewObjectArray(eventValues.Num(), FJavaWrapper::JavaStringClass, NULL);
            
            CreateJavaKeyValueArrays(Env, jKeys, jValues, eventKeys, eventValues);
            
            jstring jEventName = Env->NewStringUTF(TCHAR_TO_UTF8(*scope));
            jboolean jWithCallbackSetup = withCallbackSetup;
            
            FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jEventName, jKeys, jValues, jWithCallbackSetup);
            
            Env->DeleteLocalRef(jKeys);
            Env->DeleteLocalRef(jValues);
            Env->DeleteLocalRef(jEventName);
        }
#endif
    }
    else
    {
        UE_LOG(LogUpsight, Log, TEXT("keys and/or value arguments are empty or nil"));
    }
}

void UUpsightFunctions::UpsightRecordMonetizationEvent(int resolution, FString productID, int quantity, FString currency, float price, TArray<FString> keys, TArray<FString> values)
{
#if PLATFORM_IOS
    NSDictionary *p = CreateNSDictionary(keys, values);
    
    [Upsight recordMonetizationEventWithResolution:(USPurchaseResolution)resolution
                                          currency:currency.GetNSString()
                                          quantity:quantity
                                           product:productID.GetNSString()
                                             price:price
                                        properties:p];

#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightRecordMonetizationEvent",
                                                           "(ILjava/lang/String;ILjava/lang/String;F[Ljava/lang/String;[Ljava/lang/String;)V",
                                                           false);

        jobjectArray jKeys   = (jobjectArray)Env->NewObjectArray(keys.Num(),   FJavaWrapper::JavaStringClass, NULL);
        jobjectArray jValues = (jobjectArray)Env->NewObjectArray(values.Num(), FJavaWrapper::JavaStringClass, NULL);
        
        CreateJavaKeyValueArrays(Env, jKeys, jValues, keys, values);
        
        jint     jResolution = (jint)resolution;
        jstring  jProductID  = Env->NewStringUTF(TCHAR_TO_UTF8(*productID));
        jint     jQuantity   = (jint)quantity;
        jstring  jCurrency   = Env->NewStringUTF(TCHAR_TO_UTF8(*currency));
        jfloat   jPrice      = (jfloat)price;
        
        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jResolution, jProductID, jQuantity, jCurrency, jPrice, jKeys, jValues);
        
        Env->DeleteLocalRef(jKeys);
        Env->DeleteLocalRef(jValues);
        
        Env->DeleteLocalRef(jProductID);
        Env->DeleteLocalRef(jCurrency);
    }
#endif
}

void UUpsightFunctions::UpsightRecordMonetizationEventWithTotalPrice(int resolution, FString productID, int quantity, FString currency, float price, float totalPrice, TArray<FString> keys, TArray<FString> values)
{
#if PLATFORM_IOS
    NSDictionary *p = CreateNSDictionary(keys, values);
    
    [Upsight recordMonetizationEventWithResolution:(USPurchaseResolution)resolution totalPrice:totalPrice currency:currency.GetNSString() product:productID.GetNSString() properties:p];
#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightRecordMonetizationEventWithTotalPrice",
                                                           "(ILjava/lang/String;ILjava/lang/String;FF[Ljava/lang/String;[Ljava/lang/String;)V",
                                                           false);

        jobjectArray jKeys   = (jobjectArray)Env->NewObjectArray(keys.Num(),   FJavaWrapper::JavaStringClass, NULL);
        jobjectArray jValues = (jobjectArray)Env->NewObjectArray(values.Num(), FJavaWrapper::JavaStringClass, NULL);
        
        CreateJavaKeyValueArrays(Env, jKeys, jValues, keys, values);

        jint     jResolution = (jint)resolution;
        jstring  jProductID  = Env->NewStringUTF(TCHAR_TO_UTF8(*productID));
        jint     jQuantity   = (jint)quantity;
        jstring  jCurrency   = Env->NewStringUTF(TCHAR_TO_UTF8(*currency));
        jfloat   jPrice      = (jfloat)price;
        jfloat   jTotalPrice = (jfloat)totalPrice;

        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jResolution, jProductID, jQuantity, jCurrency, jPrice, jTotalPrice, jKeys, jValues);
        
        Env->DeleteLocalRef(jKeys);
        Env->DeleteLocalRef(jValues);
        
        Env->DeleteLocalRef(jProductID);
        Env->DeleteLocalRef(jCurrency);
    }
#endif
}

void UUpsightFunctions::UpsightRecordInAppPurchaseEventWithResolution(int resolution, FString productID, int quantity, FString currency, float price, float totalPrice, FString store, FString bundle, FString transactionIdentifier, TArray<FString> keys, TArray<FString> values)
{
     UE_LOG(LogUpsight, Log, TEXT("UpsightRecordInAppPurchaseEventWithResolution - resolution: %d, productId: %s, quantity: %d, currency: %s, price: %f, totalPrice: %f, store: %s, bundle: %s, transactionId: %s"), resolution, *productID, quantity, *currency, price, totalPrice, *store, *bundle, *transactionIdentifier);

#if PLATFORM_IOS
    NSDictionary *p = CreateNSDictionary(keys, values);
    
    NSString *nsProductId     = productID.GetNSString();
    NSString *nsCurrencyId    = currency.GetNSString();
    NSString *nsTransactionId = transactionIdentifier.GetNSString();
    
    [Upsight recordInAppPurchaseEventWithResolution:(USPurchaseResolution)resolution
                                            product:nsProductId
                                           quantity:quantity
                                              price:price
                                           currency:nsCurrencyId
                              transactionIdentifier:nsTransactionId
                                         properties:p];
    
#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightRecordInAppPurchaseEvent",
                                                           "(ILjava/lang/String;ILjava/lang/String;FFLjava/lang/String;Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
                                                           false);

        jobjectArray jKeys   = (jobjectArray)Env->NewObjectArray(keys.Num(),   FJavaWrapper::JavaStringClass, NULL);
        jobjectArray jValues = (jobjectArray)Env->NewObjectArray(values.Num(), FJavaWrapper::JavaStringClass, NULL);
        
        CreateJavaKeyValueArrays(Env, jKeys, jValues, keys, values);
        
        jint    jResolution = (jint)resolution;
        jstring jProductID  = Env->NewStringUTF(TCHAR_TO_UTF8(*productID));
        jint    jQuantity   = (jint)quantity;
        jstring jCurrency   = Env->NewStringUTF(TCHAR_TO_UTF8(*currency));
        jfloat  jPrice      = (jfloat)price;
        jfloat  jTotalPrice = (jfloat)totalPrice;
        jstring jStore      = Env->NewStringUTF(TCHAR_TO_UTF8(*store));
        jstring jBundle     = Env->NewStringUTF(TCHAR_TO_UTF8(*bundle));

        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jResolution, jProductID, jQuantity, jCurrency, jPrice, jTotalPrice, jStore, jBundle, jKeys, jValues);
        
        Env->DeleteLocalRef(jKeys);
        Env->DeleteLocalRef(jValues);
        
        Env->DeleteLocalRef(jStore);
        Env->DeleteLocalRef(jBundle);
        Env->DeleteLocalRef(jProductID);
        Env->DeleteLocalRef(jCurrency);
    }
#endif
}

void UUpsightFunctions::UpsightBillboardForScopeRegisterForCallbacks(FString scope)
{
    UE_LOG(LogUpsight, Log, TEXT("UUpsightFunctions::UpsightBillboardForScopeRegisterForCallbacks - scope '%s'"), *scope);
    
#if PLATFORM_IOS
    dispatch_async(dispatch_get_main_queue(), ^{
        id<USBillboard> billboard = [Upsight billboardForScope:scope.GetNSString()];

        billboard.delegate = ufd;
    });
    
#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightBillboardForScopeRegisterForCallbacks",
                                                           "(Ljava/lang/String;)V",
                                                           false);
        
        jstring jScope = Env->NewStringUTF(TCHAR_TO_UTF8(*scope));
        
        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jScope);
        
        Env->DeleteLocalRef(jScope);
    }
#else
     UE_LOG(LogUpsight, Log, TEXT("UpsightBillboardForScopeRegisterForCallbacks - not supported on this platform"));
#endif
}

void UUpsightFunctions::UpsightBillboardForScopeUnregisterForCallbacks(FString scope)
{
    UE_LOG(LogUpsight, Log, TEXT("UUpsightFunctions::UpsightBillboardForScopeUnregisterForCallbacks - scope '%s'"), *scope);
    
#if PLATFORM_IOS
    dispatch_async(dispatch_get_main_queue(), ^{
        id<USBillboard> billboard = [Upsight billboardForScope:scope.GetNSString()];
        
        billboard.delegate = nil;
    });
    
#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightBillboardForScopeUnregisterForCallbacks",
                                                           "(Ljava/lang/String;)V",
                                                           false);
        
        jstring jScope = Env->NewStringUTF(TCHAR_TO_UTF8(*scope));
        
        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jScope);
        
        Env->DeleteLocalRef(jScope);
    }
#else
    UE_LOG(LogUpsight, Log, TEXT("UpsightBillboardForScopeUnregisterForCallbacks - not supported on this platform"));
#endif
}

bool UUpsightFunctions::UpsightBillboardForScopeIsContentReady(FString scope)
{
    bool isContentReady = false;
    
#if PLATFORM_IOS
    id<USBillboard> billboard = [Upsight billboardForScope:scope.GetNSString()];
    
    if (!billboard)
    {
        UE_LOG(LogUpsight, Log, TEXT("UpsightBillboardForScopeIsContentReady - unable to find billboard for scope: %s"), *scope);
    }
    
    isContentReady = billboard.contentReady == YES ? true : false;
#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightBillboardForScopeIsContentReady",
                                                           "(Ljava/lang/String;)Z",
                                                           false);
        
        jstring jScope = Env->NewStringUTF(TCHAR_TO_UTF8(*scope));
        
        isContentReady = FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, Method, jScope);

        Env->DeleteLocalRef(jScope);
    }
#else
    UE_LOG(LogUpsight, Log, TEXT("UpsightBillboardForScopeIsContentReady - not supported on this platform"));
#endif
    
    UE_LOG(LogUpsight, Log, TEXT("UpsightBillboardForScopeIsContentReady - content is '%s'"), isContentReady ? TEXT("yes") : TEXT("no"));
    
    return isContentReady;
}

void UUpsightFunctions::UpsightBillboardUnregisterCallbacks()
{
    UE_LOG(LogUpsight, Log, TEXT("UUpsightFunctions::UpsightBillboardUnregisterCallbacks"));
    
#if PLATFORM_IOS
    
#elif PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env,
                                                           FJavaWrapper::GameActivityClassID,
                                                           "AndroidThunkJava_UpsightBillboardUnregisterCallbacks",
                                                           "()V",
                                                           false);
    }
#else
     UE_LOG(LogUpsight, Log, TEXT("UpsightBillboardUnregisterCallbacks - not supported on this platform"));
#endif
}

void UUpsightFunctions::UpsightRegisterForPushNotifications()
{
#if PLATFORM_IOS
    [USPush registerForPushNotifications];
#else
    UE_LOG(LogUpsight, Log, TEXT("UpsightRegisterForPushNotifications - not supported on this platform"));
#endif
}

#if PLATFORM_ANDROID
extern "C" void Java_com_epicgames_ue4_GameActivity_nativeUpsightBillboardOnPurchases(JNIEnv* jenv,
                                                                                      jobject thiz,
                                                                                      jobjectArray productIDs,
                                                                                      jintArray quantities)
{
    UE_LOG(LogUpsight, Log, TEXT("Java_com_epicgames_ue4_GameActivity_nativeUpsightBillboardOnPurchases"));
    
    TArray<UUpsightVirtualGoodPromotionPurchase *> Purchases;
    
    jsize jNumPurchases = jenv->GetArrayLength(productIDs);
    jint* jQuantities   = jenv->GetIntArrayElements(quantities, NULL);
    
    for (int i = 0; i < jNumPurchases; i++)
    {
        jstring jProductID = (jstring)jenv->GetObjectArrayElement(productIDs, i);
        const char* charsProductID = jenv->GetStringUTFChars(jProductID, 0);
        
        FString ProductID(FString(UTF8_TO_TCHAR(charsProductID)));
        
        jenv->ReleaseStringUTFChars(jProductID, charsProductID);
        jenv->DeleteLocalRef(jProductID);
        
        UUpsightVirtualGoodPromotionPurchase *p = NewObject<UUpsightVirtualGoodPromotionPurchase>();
        
        p->Name          = ProductID;
        p->Quantity      = jQuantities[i];

        Purchases.Add(p);
    }
    
    UUpsightComponent::DidReceieveVirtualGoodPromotionPurchaseDelegate.Broadcast(Purchases);
}

extern "C" void Java_com_epicgames_ue4_GameActivity_nativeUpsightBillboardOnRewards(JNIEnv* jenv,
                                                                                    jobject thiz,
                                                                                    jobjectArray productIDs,
                                                                                    jintArray    quantities,
                                                                                    jobjectArray signatureDatas)
{
    UE_LOG(LogUpsight, Log, TEXT("Java_com_epicgames_ue4_GameActivity_nativeUpsightBillboardOnRewards"));
    
    TArray<UUpsightReward *> Rewards;
    
    jsize jNumRewards = jenv->GetArrayLength(productIDs);
    jint* jQuantities = jenv->GetIntArrayElements(quantities, NULL);
    
    for (int i = 0; i < jNumRewards; i++)
    {
        jstring jProductID = (jstring)jenv->GetObjectArrayElement(productIDs, i);
        const char* charsProductID = jenv->GetStringUTFChars(jProductID, 0);
        
        FString ProductID(FString(UTF8_TO_TCHAR(charsProductID)));
        
        jenv->ReleaseStringUTFChars(jProductID, charsProductID);
        jenv->DeleteLocalRef(jProductID);

        jstring jSignatureData = (jstring)jenv->GetObjectArrayElement(signatureDatas, i);
        const char* charsSignatureData = jenv->GetStringUTFChars(jSignatureData, 0);
        
        FString RewardSignatureData(FString(UTF8_TO_TCHAR(charsSignatureData)));
        
        jenv->ReleaseStringUTFChars(jSignatureData, charsSignatureData);
        jenv->DeleteLocalRef(jSignatureData);
        
        UUpsightReward *r = NewObject<UUpsightReward>();
        
        r->Name          = ProductID;
        r->Quantity      = jQuantities[i];
        r->SignatureData = RewardSignatureData;
        
        Rewards.Add(r);
    }
    
    jenv->ReleaseIntArrayElements(quantities, jQuantities, 0);
    
    UUpsightComponent::DidReceieveRewardDelegate.Broadcast(Rewards);
}

extern "C" void Java_com_epicgames_ue4_GameActivity_nativeUpsightBillboardOnNextView(JNIEnv* jenv, jobject thiz)
{
    
}

extern "C" void Java_com_epicgames_ue4_GameActivity_nativeUpsightBillboardOnDetach(JNIEnv* jenv, jobject thiz)
{
    
}
#endif





