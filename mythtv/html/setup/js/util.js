
function loadSetupPage(pageName) {
    loadContent("/setup/" + pageName + ".html",
                "/setup/js/" + pageName + ".js");
}

function interceptSetupMenuClicks() {
    $('#menu ul li a').click(function(event) {
        if (this.href.match("/Config/") == "/Config/") {
            event.preventDefault();
            loadContent(this.href, "/setup/js/Config.js");
        }
    });
}

function basename(path) {
    return path.replace( /\\/g, '/').replace( /.*\//, '' );
}

function dirname(path) {
    return path.replace( /\\/g, '/').replace( /\/[^\/]*$/, '' );;
}

function parentDirName(path) {
    return basename(dirname(path));
}

function setupPageName(path) {
    return basename(path).replace( /\\/g, '/').replace( /\.[^\.]$/, '' );
}

/////////////////////////////////////////////////////////////////////////
var serverHostName = "";

function getServerHostName() {
    if (serverHostName.length == 0) {
        $.ajaxSetup({ async: false });
        $.getJSON("/Myth/GetHostName", function(data) {
            serverHostName = data.StringList.Values[0];
        });
        $.ajaxSetup({ async: true });
    }

    return serverHostName;
}

