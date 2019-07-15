// ==UserScript==
// @name          Wallapop (Responsive mode)
// @namespace     http://blog.alefnode.com
// @description	  Wallapop is now responsive
// @author        alefnode
// @version       0.20150805144242
// ==/UserScript==

var check = 0;
var checkExist = setInterval(function() {
   if (document.getElementById("conversations").childNodes.length) {
      console.log("Exists!");
      clearInterval(checkExist);
      if ( check == 0 ) {
        main();

      }
      check = 1;
   }
}, 100);



function main(){
  		document.getElementsByTagName('tsl-sidebar')[0].remove();
      document.getElementsByTagName('tsl-chat')[0].style.marginLeft = 'auto';
      document.getElementById('left-panel').style.width = 'auto';
      document.getElementById('empty-state').style.display = 'none';


      var checkExist1 = setInterval(function() {
          document.getElementById("conversations").onclick = function() {
           	console.log("Exists!");
           	clearInterval(checkExist1);
           	document.getElementById('left-panel').style.display = 'none';
           	document.getElementById('right-panel').style.display = 'none';
            document.getElementById('center-panel').getElementsByTagName('tsl-user-detail')[0].style.display = 'none';
            document.getElementById('center-panel').style.display = '';
           	document.getElementById('center-panel').style.width = 'auto';
           	document.getElementById('center-panel').style.height = 'auto';
           	document.getElementById('center-panel').style.marginLeft = 'auto';
           	document.getElementById('center-panel').style.marginRight = 'auto';
           	menu();
         }
      }, 1000);

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
    newHTML.id = "back_button";
    newHTML.style = "";
    newHTML.innerHTML   = "<button onclick=\"document.getElementById('left-panel').style.display = ''; document.getElementById('center-panel').style.display = 'none'; document.getElementById('right-panel').style.display = ''; document.getElementById('back_button').remove(); \"><span data-icon='left'><svg class='svg_back' id='Layer_1' xmlns='http://www.w3.org/2000/svg' viewBox='0 0 21 21' width='21' height='21'><path fill='#263238' fill-opacity='.33' d='M4.8 6.1l5.7 5.7 5.7-5.7 1.6 1.6-7.3 7.2-7.3-7.2 1.6-1.6z'></path></svg></span></button>";

    document.body.appendChild(newHTML);
    //var eElement = document.getElementsByTagName("tsl-topbar").getElementsByTagName("DIV");
    //eElement.insertBefore(newHTML, eElement.firstChild);

    check = check + 1;
  }

}
