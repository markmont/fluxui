chrome.app.runtime.onLaunched.addListener(function() {
  chrome.app.window.create('../html/fluxui.html',
    {width: 500, height: 600});
});

