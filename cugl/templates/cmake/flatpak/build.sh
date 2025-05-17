#!/bin/sh
BASEDIR=$(dirname $0)
ROOTDIR=${BASEDIR}/..
INSTDIR=${BASEDIR}/../install

# Notify the user of the correct script usage
warn() {
    echo "usage: build.sh2 [clean]"
}

# Remove the repo and flatpak build directories
clean() {
    echo "Cleaning flatpak directory"
    rm -rf ${BASEDIR}/repo
    rm -rf ${BASEDIR}/cache
    rm -rf ${ROOTDIR}/repo
    rm -rf ${ROOTDIR}/cache
}

# Create the flatpak installation
build() {
    echo "Building __APP_ID__.flatpak"

    # Create the flatpak
    flatpak-builder --repo=repo --force-clean cache ${BASEDIR}/__APP_ID__.yml
    flatpak build-bundle repo ${BASEDIR}/__APP_ID__.flatpak __APP_ID__
    rm -rf ${INSTDIR}
    mkdir ${INSTDIR}
    mv ${BASEDIR}/__APP_ID__.flatpak ${INSTDIR}/

    # Create the Steam shortcut
    echo "#!/bin/sh" >> "${INSTDIR}/__SHORTCUT__.exe"
    echo "flatpak run __APP_ID__" >> "${INSTDIR}/__SHORTCUT__.exe"
    chmod a+x "${INSTDIR}/__SHORTCUT__.exe"

    # Create the flatpak installer
    echo "#!/bin/sh" >> ${INSTDIR}/install.sh
    echo "BASEDIR=\$(dirname \$0)" >> ${INSTDIR}/install.sh
    echo "flatpak install \${BASEDIR}/__APP_ID__.flatpak " >> ${INSTDIR}/install.sh
    chmod a+x ${INSTDIR}/install.sh

    # Create the flatpak uninstaller
    echo "#!/bin/sh" >> ${INSTDIR}/uninstall.sh
    echo "flatpak uninstall __APP_ID__ " >> ${INSTDIR}/uninstall.sh
    chmod a+x ${INSTDIR}/uninstall.sh

    echo "Files written to ${INSTDIR}"
}


# Main script
if [ $# -gt 1 ]
then
    warn
elif [ $# -eq 1 ]
then
    if [ $1 = "clean" ]
    then
        clean
    else
        warn
    fi
else
    build
fi
