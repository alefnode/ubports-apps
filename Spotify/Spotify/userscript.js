var css = ["html, body { font-family: Ubuntu !important; background: #141526 !important; color: #9193a8; }",
        ".speed-results-container.succeeded { color: #fff; }",
        ".speed-units-container.succeeded { color: #9193a8; }",
        ".your-speed-message { color: #9193a8; }",
        ".speed-progress-indicator.succeeded { border-color: #1cbfff; color: #9193a8; }",
        ".speed-progress-indicator.in-progress > .spinner { box-shadow: 0 .3vh 0 0 #1cbfff; }",
        ".logo { display: none; }",
        ".footer-container { display: none; }",
        ""].join("\n");

var node = document.createElement("style");
node.type = "text/css";
node.appendChild(document.createTextNode(css));
var heads = document.getElementsByTagName("head");
if (heads.length > 0) {
    heads[0].appendChild(node);
} else {
    document.documentElement.appendChild(node);
}

document.getElementsByClassName("share-container")[0].remove();
document.getElementsByClassName("ookla-container")[0].remove();

document.getElementsByClassName("footer-container")[0].style.display = "none"
document.getElementsByClassName("logo")[0].style.display = "none"
