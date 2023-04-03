// ==UserScript==
// @name          WhatsApp (Responsive mode)
// @namespace     http://blog.alefnode.com
// @description	  Whats App is now responsive
// @author        alefnode
// @version       0.20150805144242
// ==/UserScript==

// Declare variables
updatenotificacion = 0;
allownotification = 0;


// Listeners to startup APP
window.addEventListener("load", function(event) {
    console.log("Loaded");
    main();
});

window.onload = (event) => {
  console.log('page is fully loaded');
};

document.addEventListener('readystatechange', event => {
    console.log(event.target.readyState);
    if (event.target.readyState === "complete") {
        console.log("Completed");
    }
});

// First resize after loading the web
var check = 0;
var checkExist = setInterval(function() {
  if (document.getElementById('app').getElementsByClassName('browser')[0]) {
    clean();
    location.reload();
  } else {
    if (document.getElementById('app').getElementsByClassName('landing-wrapper').length) {
      document.getElementById('app').getElementsByClassName('landing-wrapper')[0].style.minWidth = 'auto';
      document.getElementById('app').getElementsByClassName('landing-header')[0].style.display = 'none';
    }
    if (document.getElementById("app").getElementsByClassName('two')[0].childNodes.length) {
      console.log("Exists!");
      if ( check == 0 ) {
        clearInterval(checkExist);
        clean();
        main();
      }
      check = 1;
    }
  }
}, 200);

// Analize JS after every click on APP and execute Actions
window.addEventListener("click", function() {
  console.log("Click");

  if (document.getElementById("app").getElementsByClassName('two')[0].childNodes[2].style.display == 'none') {
    navigation();
  }

  if (document.querySelector('span[data-icon="attach-image"]')){
    attachresponsive();
  } else if (document.querySelector('[data-animate-dropdown-item]')){
    modaldialogresponsive();
  } else if (document.querySelector('[data-testid="contact-list-key"]')){
    startnewchat();
  }

  if (updatenotificacion == 0 || allownotification == 0){
    disablenotifications();
  }

});

// Define all the functions to work on it
function main(){
  console.log("Call main function")
  document.getElementById("app").getElementsByClassName('two')[0].childNodes[4].style.display = 'none';
  document.getElementById("app").getElementsByClassName('two')[0].childNodes[2].childNodes[1].style.display = 'none';
  document.getElementById("app").getElementsByClassName('two')[0].childNodes[3].style.minWidth = "100%"
  document.getElementById('app').getElementsByClassName('two')[0].style.minWidth = 'auto';
  document.getElementById('app').getElementsByClassName('two')[0].style.minHeight = 'auto';

  var elems = document.getElementById("pane-side").getElementsByTagName("DIV");
  for (var i = 0; i<elems.length; i++) {
    elems[i].onclick = function() {

      document.getElementById("app").getElementsByClassName('two')[0].childNodes[2].childNodes[1].style.display = '';
      document.getElementById("app").getElementsByClassName('two')[0].childNodes[4].style.display = '';
      document.getElementById("app").getElementsByClassName('two')[0].childNodes[3].style.display = 'none';
      menu();

    };
  }

  disablenotifications();

}


function navigation() {
  var check = 0;
  var checkExist = setInterval(function() {
    if (document.getElementById("app").getElementsByClassName('two')[0].childNodes[3].style.display == 'none') {
      console.log("Exists!");
      if ( check == 0 ) {
        clearInterval(checkExist);
        menu();
      }
      check = 1;
    }
  }, 200);
}

function menu(){

  console.log("Call menu function")
  function addCss(cssString) {
      var head = document.getElementsByTagName('head')[0];
      var newCss = document.createElement('style');
      newCss.type = "text/css";
      newCss.innerHTML = cssString;
      head.appendChild(newCss);
  }
  function addJS(jsString) {
      var head = document.getElementsByTagName('head')[0];
      var newJS = document.createElement('script');
      newJS.innerHTML = jsString;
      head.appendChild(newJS);
  }

  check = 0;
  if ( check == 0 ) {
    addCss(".back_button a { display:block; height: 100%; width: 100%;}.back_button { position: absolute; left: 0; z-index:200; width:60px; height:45px; display:-webkit-flex; display:flex; -webkit-align-items:center; align-items:center; -webkit-justify-content:center; justify-content:center } html[dir] .back_button { border-radius:50%; } html[dir=ltr] .back_button { right:11px } html[dir=rtl] .back_button { left:11px } .back_button path { fill:#93999c; fill-opacity:1 } .svg_back { transform: rotate(90deg); height: 100%;}");

  	addJS('window.onscroll = function() {myFunction()}; var navbar = document.getElementById("navbar"); var sticky = navbar.offsetTop; function myFunction() { if (window.pageYOffset >= sticky) { navbar.classList.add("sticky") } else { navbar.classList.remove("sticky"); } } ');

    var newHTML         = document.createElement('div');
    newHTML.className += "back_button";
    newHTML.style = "";
    newHTML.innerHTML   = "<a href='#' onclick=\"document.getElementById('app').getElementsByClassName('two')[0].childNodes[4].style.display = 'none';\
                                                 document.getElementById('app').getElementsByClassName('two')[0].childNodes[3].style.display = 'block';\
                                                 document.getElementById('app').getElementsByClassName('two')[0].childNodes[2].childNodes[1].style.display = 'none';\
                          \"><span data-icon='left'><svg class='svg_back' id='Layer_1' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 21 21' width='21' height='21'><path fill='#263238' fill-opacity='.33' d='M4.8 6.1l5.7 5.7 5.7-5.7 1.6 1.6-7.3 7.2-7.3-7.2 1.6-1.6z'></path></svg></span></a>";

    var eElement = document.getElementById("main").childNodes[1];
    eElement.insertBefore(newHTML, eElement.firstChild);

    check = check + 1;
  }

}

function disablenotifications(){
  // Disable update available notification
  if (document.querySelector('span[data-icon="alert-update"]')) {
    document.querySelector('span[data-icon="alert-update"]').parentElement.parentElement.style.display = 'none';
    console.log("Disabled update available notification");
    updatenotification = 1;
  }
  // Disable request to allow notifications
  if (document.querySelector('span[data-icon="alert-notification"]')) {
    document.querySelector('span[data-icon="alert-notification"]').parentElement.parentElement.style.display = 'none';
    console.log("Disabled request allow notification");
    allownotification = 1;
  }
}

function attachresponsive(){
  if (document.querySelector('span[data-icon="attach-image"]')){
    var check = 0;
    var checkExist = setInterval(function() {
     	if (document.getElementById('app').getElementsByClassName('two')[0].childNodes[2].childNodes[1].children[0].children.length > 0) {
       	// Hide chat to resize attach image panel
      	document.getElementById('app').getElementsByClassName('two')[0].childNodes[2].childNodes[0].style.display = 'none';
        document.getElementById('app').getElementsByClassName('two')[0].querySelector("input").parentElement.style.minWidth = "0px";

      	if ( check == 0 ) {
        	clearInterval(checkExist);
        }
      }
      check = 1;
  	}, 500);
	} else {
    document.getElementById('app').getElementsByClassName('two')[0].childNodes[2].childNodes[0].style.display = '';
    document.getElementById('app').getElementsByClassName('two')[0].querySelector("input").parentElement.style.minWidth = "";
  }
}

function modaldialogresponsive(){
  if (document.querySelector('[data-animate-dropdown-item]')){
    var check = 0;
    var checkExist = setInterval(function() {
     	if (document.querySelector("[data-animate-modal-backdrop]")) {
       	// Delete min-width class to center dialog message
      	document.querySelector("[data-animate-modal-backdrop]").childNodes[0].style.minWidth = "0px";

      	if ( check == 0 ) {
        	clearInterval(checkExist);
        }
      }
      check = 1;
  	}, 300);
	} else {
    document.querySelector("[data-animate-modal-backdrop]").childNodes[0].style.minWidth = "";
  }
}

function startnewchat(){
  var elems = document.querySelector('[data-testid="contact-list-key"]').getElementsByTagName("DIV");
  for (var i = 0; i<elems.length; i++) {
    elems[i].onclick = function() {

      document.getElementById("app").getElementsByClassName('two')[0].childNodes[2].childNodes[1].style.display = '';
      document.getElementById("app").getElementsByClassName('two')[0].childNodes[4].style.display = '';
      document.getElementById("app").getElementsByClassName('two')[0].childNodes[3].style.display = 'none';
      menu();

    };
  }
}

function clean() {
  navigator.serviceWorker.getRegistrations().then(function(registrations) {
  for(let registration of registrations) {
          registration.unregister()
  }}).catch(function(err) {
      console.log('Service Worker registration failed: ', err);
  });
}
