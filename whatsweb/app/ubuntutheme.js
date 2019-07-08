// ==UserScript==
// @name          WhatsApp (Responsive mode)
// @namespace     http://blog.alefnode.com
// @description	  Whats App is now responsive
// @author        alefnode
// @version       0.20150805144242
// ==/UserScript==



/*
      html_orig = document.getElementById('app').innerHTML
      side_orig = document.getElementById('side').innerHTML
      html = document.getElementById('app')
      side = document.getElementById('side').parentNode
      html.innerHTML = side.innerHTML
			document.getElementById('side').parentNode.style.display = 'block';
*/


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

var check = 0;
var checkExist = setInterval(function() {
   if (document.getElementById('app').getElementsByClassName('landing-wrapper').length) {
  		document.getElementById('app').getElementsByClassName('landing-wrapper')[0].style.minWidth = 'auto';
			document.getElementById('app').getElementsByClassName('landing-header')[0].style.display = 'none';
   }
   if (document.getElementById("app").getElementsByClassName('app two')[0].childNodes.length) {
      console.log("Exists!");
      clearInterval(checkExist);
      if ( check == 0 ) {
        main();

      }
      check = 1;
   }
}, 100);



function main(){
      //document.getElementById('app').getElementsByClassName('_3q4NP _1Iexl')[1].style.display = 'none';
      document.getElementById("app").getElementsByClassName('app two')[0].childNodes[3].style.display = 'none';
      document.getElementById('app').getElementsByClassName('app two')[0].style.minWidth = 'auto';

      var elems = document.getElementById("pane-side").getElementsByTagName("DIV");
      for (var i = 0; i<elems.length; i++) {
        elems[i].onclick = function() {

          //document.getElementById('app').getElementsByClassName('_3q4NP _1Iexl')[1].style.display = 'block';
          //document.getElementById('app').getElementsByClassName('_3q4NP k1feT')[1].style.display = 'none';
          document.getElementById("app").getElementsByClassName('app two')[0].childNodes[3].style.display = '';
          document.getElementById("app").getElementsByClassName('app two')[0].childNodes[2].style.display = 'none';

          menu();

          /*
          var button = document.createElement("Button");
          button.innerHTML = 'BACK';
          button.style = "top:50;left:0;position:absolute;z-index: 9999; -moz-box-shadow:inset 0px 1px 0px 0px #ffffff;-webkit-box-shadow:inset 0px 1px 0px 0px #ffffff;box-shadow:inset 0px 1px 0px 0px #ffffff;background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ededed), color-stop(1, #dfdfdf));background:-moz-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:-webkit-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:-o-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:-ms-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:linear-gradient(to bottom, #ededed 5%, #dfdfdf 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ededed', endColorstr='#dfdfdf',GradientType=0);background-color:#ededed;border:1px solid #dcdcdc;display:inline-block;cursor:pointer;color:#777777;font-family:Arial;font-size:15px;font-weight:bold;padding:6px 8px;text-decoration:none;text-shadow:0px 1px 0px #ffffff;";
          button.onclick = function() { document.getElementById("app").getElementsByClassName('app two')[0].childNodes[3].style.display = 'none'; document.getElementById("app").getElementsByClassName('app two')[0].childNodes[2].style.display = 'block'; }
          document.body.appendChild(button);
          */

        };
      }

}


function menu(){


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
    addCss(".back_button { position:absolute; left: 5px; bottom:50%; z-index:200; width:42px; height:42px; display:-webkit-flex; display:flex; -webkit-align-items:center; align-items:center; -webkit-justify-content:center; justify-content:center } html[dir] .back_button { border-radius:50%; background-color:#fff; box-shadow:0 1px 1px 0 rgba(0,0,0,.06),0 2px 5px 0 rgba(0,0,0,.2) } html[dir=ltr] .back_button { right:11px } html[dir=rtl] .back_button { left:11px } .back_button path { fill:#93999c; fill-opacity:1 } .svg_back { transform: rotate(90deg); }");

  	addJS('window.onscroll = function() {myFunction()}; var navbar = document.getElementById("navbar"); var sticky = navbar.offsetTop; function myFunction() { if (window.pageYOffset >= sticky) { navbar.classList.add("sticky") } else { navbar.classList.remove("sticky"); } } ');

    var newHTML         = document.createElement('div');
    newHTML.className += "back_button";
    newHTML.style = "";
    newHTML.innerHTML   = "<a href='#' onclick=\"document.getElementById('app').getElementsByClassName('app two')[0].childNodes[3].style.display = 'none'; document.getElementById('app').getElementsByClassName('app two')[0].childNodes[2].style.display = 'block'; \"><span data-icon='left'><svg class='svg_back' id='Layer_1' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 21 21' width='21' height='21'><path fill='#263238' fill-opacity='.33' d='M4.8 6.1l5.7 5.7 5.7-5.7 1.6 1.6-7.3 7.2-7.3-7.2 1.6-1.6z'></path></svg></span></a>";

    //document.body.appendChild(newHTML);
    var eElement = document.getElementById("app").getElementsByClassName('copyable-area')[0];
    eElement.insertBefore(newHTML, eElement.firstChild);

    check = check + 1;
  }

}
