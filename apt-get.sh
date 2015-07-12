
arch=`arch`

while [ $# -gt 0 ]; do
    case "$1" in
	armhf)
	    echo "*********** apt-get armhf"
	    arch='armhf'
	    shift
	    ;;
	*)
	    break
	    ;;
    esac
done

apt-get install qt5-default:$arch
apt-get install libqt5svg5-dev:$arch
apt-get install libqt5script5-dev:$arch
apt-get install libqt5xmlpatterns5-dev:$arch
apt-get install libqt5webkit5-dev:$arch
apt-get install libqt5webkit5-dev:$arch
apt-get install qtxmlpatterns5-dev-tools:$arch
apt-get install libxml2-dev:$arch
apt-get install libxslt1-dev:$arch
