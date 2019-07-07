var checkExist = setInterval(function() {
    if (document.getElementById('app').getElementsByClassName('landing-wrapper').length) {
       document.getElementById('app').getElementsByClassName('landing-wrapper')[0].style.minWidth = 'auto';
       document.getElementById('app').getElementsByClassName('landing-header')[0].style.display = 'none';
    }
    if (document.getElementById("app").getElementsByClassName('app two')[0].childNodes.length) {
       console.log("Exists!");
       main();
       clearInterval(checkExist);
    }
}, 100);



function main(){
      //document.getElementById('app').getElementsByClassName('_3q4NP _1Iexl')[1].style.display = 'none';
      document.getElementById("app").getElementsByClassName('app two')[0].childNodes[3].style.display = 'none';
      document.getElementById('app').getElementsByClassName('app two')[0].style.minWidth = 'auto';

      var elems = document.getElementById('app').getElementsByClassName('_3La1s');
      for (var i = 0; i<elems.length; i++) {
        elems[i].onclick = function() {

          //document.getElementById('app').getElementsByClassName('_3q4NP _1Iexl')[1].style.display = 'block';
          //document.getElementById('app').getElementsByClassName('_3q4NP k1feT')[1].style.display = 'none';
          document.getElementById("app").getElementsByClassName('app two')[0].childNodes[3].style.display = 'block';
          document.getElementById("app").getElementsByClassName('app two')[0].childNodes[2].style.display = 'none';
          var button = document.createElement("Button");
          button.innerHTML = "BACK";
          button.style = "top:0;right:0;position:absolute;z-index: 9999; -moz-box-shadow:inset 0px 1px 0px 0px #ffffff;-webkit-box-shadow:inset 0px 1px 0px 0px #ffffff;box-shadow:inset 0px 1px 0px 0px #ffffff;background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ededed), color-stop(1, #dfdfdf));background:-moz-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:-webkit-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:-o-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:-ms-linear-gradient(top, #ededed 5%, #dfdfdf 100%);background:linear-gradient(to bottom, #ededed 5%, #dfdfdf 100%);filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ededed', endColorstr='#dfdfdf',GradientType=0);background-color:#ededed;border:1px solid #dcdcdc;display:inline-block;cursor:pointer;color:#777777;font-family:Arial;font-size:15px;font-weight:bold;padding:6px 8px;text-decoration:none;text-shadow:0px 1px 0px #ffffff;";
          button.onclick = function() { document.getElementById("app").getElementsByClassName('app two')[0].childNodes[3].style.display = 'none'; document.getElementById("app").getElementsByClassName('app two')[0].childNodes[2].style.display = 'block'; }
          document.body.appendChild(button);


        };
      }
}
