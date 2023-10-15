# exit(1) if any subcommands fail
set -e

PROJECT_NAME=$1

if [ -z "${PROJECT_NAME}" ]; then
	echo "argument 1($1) expected to be project name"
	exit 1
fi

# build temporary dir
BUILD_DIR="./build"
#	use BUILT_PRODUCTS_DIR for PopAction_Apple which gets first stdout output
BUILD_UNIVERSAL_DIR=${BUILT_PRODUCTS_DIR}

BUILDPATH_IOS="${BUILD_DIR}/${PROJECT_NAME}_Ios"
BUILDPATH_SIM="${BUILD_DIR}/${PROJECT_NAME}_IosSimulator"
BUILDPATH_MACOS="${BUILD_DIR}/${PROJECT_NAME}_Macos"
BUILDPATH_TVOS="${BUILD_DIR}/${PROJECT_NAME}_Tvos"

SCHEME_IOS=${PROJECT_NAME}_IosFramework

PRODUCT_NAME_UNIVERSAL="${PROJECT_NAME}.xcframework"

CONFIGURATION="Release"
DESTINATION_IOS="generic/platform=iOS"

# build archived frameworks
# see https://github.com/NewChromantics/PopAction_BuildApple/blob/master/index.js for some battle tested CLI options
echo "Building sub-framework archives..."
xcodebuild archive -scheme ${SCHEME_IOS} -archivePath $BUILDPATH_IOS SKIP_INSTALL=NO -sdk iphoneos -configuration ${CONFIGURATION} -destination ${DESTINATION_IOS}
#xcodebuild archive -scheme ${PROJECT_NAME}_IosSimulator -archivePath $BUILDPATH_SIM SKIP_INSTALL=NO -sdk iphonesimulator
#xcodebuild archive -scheme ${PROJECT_NAME}_Macos -archivePath $BUILDPATH_MACOS SKIP_INSTALL=NO
#xcodebuild archive -scheme ${PROJECT_NAME}_Tvos -archivePath $BUILDPATH_TVOS SKIP_INSTALL=NO



# bundle together
echo "Building xcframework..."
#xcodebuild -create-xcframework -framework ${BUILDPATH_IOS}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Ios.framework -framework ${BUILDPATH_SIM}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Ios.framework -framework ${BUILDPATH_OSX}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Osx.framework -output ./build/${FULL_PRODUCT_NAME}
xcodebuild -create-xcframework -framework ${BUILDPATH_IOS}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Ios.framework -output ${BUILD_UNIVERSAL_DIR}/${PRODUCT_NAME_UNIVERSAL}
#xcodebuild -create-xcframework -framework ${BUILDPATH_IOS}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Ios.framework -framework ${BUILDPATH_SIM}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Ios.framework -framework ${BUILDPATH_OSX}.xcarchive/Products/Library/Frameworks/${PROJECT_NAME}_Osx.framework -output ${BUILT_PRODUCTS_DIR}/${FULL_PRODUCT_NAME}

echo "xcframework ${PRODUCT_NAME_UNIVERSAL} built successfully"

# output meta for https://github.com/NewChromantics/PopAction_BuildApple
# which matches a regular scheme output
echo "FULL_PRODUCT_NAME=${PRODUCT_NAME_UNIVERSAL}"
echo "BUILT_PRODUCTS_DIR=${BUILD_UNIVERSAL_DIR}"

# output meta for github
# gr: I think my apple action will pull this out? if we use FULL_PRODUCT_NAME?
#echo PRODUCT_NAME=${PRODUCT_NAME_UNIVERSAL} >> GITHUB_OUTPUT
#echo PRODUCT_PATH=${BUILD_DIR} >> GITHUB_OUTPUT

