window.addEventListener('readystatechange', oxide_dom, true);

function oxide_dom() {
    var w1 = document.createElement('webview');
    var headerRemoveRule = {
      conditions: [
        new chrome.webViewRequest.RequestMatcher()
      ],
      actions: [
        new chrome.webViewRequest.RemoveResponseHeader({
          name: 'x-robots-tag'
        })
      ]
    };

    // declarative WebRequest API, call before loading webview.
    w1.request.onRequest.addRules([headerRemoveRule]);
    w1.src = 'https://jsbin.com/piwakil';
    document.body.appendChild(w1);
}
