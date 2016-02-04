//
//  UpsightServerEventsSupport.h
//  UpsightKit
//
//  Created by Tolik Shevchenko on 8/11/15.
//  Copyright (c) 2015 Upsight. All rights reserved.
//

#import "Upsight.h"

@interface Upsight (ServerEventsSupport)

/** Provides read-only access to a unique number generated by the SDK. This number is used to identify the device to the Upsight servers. It will only change if the user uninstalls the application and then reinstalls it. */
+ (NSString *)senderID;

@end
