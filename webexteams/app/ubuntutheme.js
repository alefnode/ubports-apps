// ==UserScript==
// @name          Webext Teams (Responsive mode)
// @namespace     http://blog.alefnode.com
// @description	  Webex Teams is now responsive
// @author        alefnode
// @version       0.20150805144242
// ==/UserScript==


var checkExist = setInterval(function() {
   if (document.getElementById('conversation-list').childNodes.length) {
      console.log('Exists!');
      main();
      clearInterval(checkExist);
   }
}, 100);


function main(){
      document.getElementById('activity-section').style.display = 'none';
			document.getElementById('convo-section').style.display = 'block';

      var elems = document.getElementById('conversation-list').childNodes;
      for (var i = 0; i<elems.length; i++) {
        elems[i].onclick = function() {

          document.getElementById('activity-section').style.display = '';
					document.getElementById('convo-section').style.display = 'none';
          var button = document.createElement("Button");
          button.innerHTML = "BACK";
          button.id = "back_button";
          button.className += "md-button md-button--none navigation-list-item-button";
          button.style = "color: white; top:0;right:0;position:absolute;z-index: 9999;";
          button.onclick = function() { document.getElementById('activity-section').style.display = 'none';document.getElementById('convo-section').style.display = 'block';  var elem = document.getElementById('back_button'); document.getElementById('container').getElementsByClassName('navigation-list-item--bottom')[0].removeChild(elem);}
          document.getElementById('container').getElementsByClassName('navigation-list-item--bottom')[0].appendChild(button);


        };
      }
}
