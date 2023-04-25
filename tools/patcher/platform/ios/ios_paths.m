#import <Foundation/Foundation.h>

#include "ios_paths.h"

#ifdef __cplusplus
extern "C" {
#endif

char *IOSGetPrefPath()
{
	NSArray *array = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);

	const char *base = "";
	if ([array count] > 0) {
		NSString *str = [array objectAtIndex:0];
		str = [str stringByAppendingString:@"/"];
		base = [str fileSystemRepresentation];
	}
	char *copy = malloc(strlen(base) + 1);
	strcpy(copy, base);
	return copy;
}

#ifdef __cplusplus
}
#endif
