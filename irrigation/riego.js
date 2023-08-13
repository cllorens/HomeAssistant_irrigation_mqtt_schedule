// asign variables to svg objects
// pumps
var bombas=[];
for (var n=1;n<=2;n++) bombas[n] = document.getElementById('BOMBA'+n);

// pumps plungers
var embolos=[];
for (var n=1;n<=2;n++) embolos[n] = document.getElementById('EMBOLO'+n);

// pumps plungers
var circulos=[];
for (var n=1;n<=2;n++) circulos[n] = document.getElementById('CIRCULO'+n);

// Vasija
var agua=document.getElementById("AGUA");
var aguafondo=document.getElementById("FONDO_AGUA");
var led1=document.getElementById("LED1");
var led2=document.getElementById("LED2");
mueveagua(0);

//Global variables
var angs=[0,0];
var pumpsm=[0,0];

//Pumps movement periodic call
setInterval(moverbombas,100);

if (!!window.EventSource)
  {
  var source = new EventSource("eventSensors.php?dummy="+Math.random());
  source.addEventListener("message", function(e) {readstatus(e.data);}, false);
  source.addEventListener("open", function(e) {console.log("OPENED STATUS EVENT");}, false);
  source.addEventListener("error", function(e) {console.log("ERROR STATUS EVENT");}, false);
  } else alert("EventSource not supported!");
  
/*****************************************/
// MAIN EVENT FUNCTION
/*****************************************/
function readstatus(res)
	{	
	// if correct answer
	if (res.indexOf("KO")!=-1) return;

	// Parse differential JSON
	var Status = JSON.parse(res);
	
	console.log(Status);
	
	pumpsm[0]=Status["Bomba 1"]=="On";
	pumpsm[1]=Status["Bomba 2"]=="On";

	var llenado=parseInt(Status["Vasija"]);
	rgb=HSVtoRGB((llenado*0.33)/100,1,1);
	led1.style.fill='rgb('+rgb.r+','+rgb.g+','+rgb.b+')';
	led2.style.fill='rgb('+rgb.r+','+rgb.g+','+rgb.b+')';
	mueveagua(llenado);
	}

/*****************************************/
// Pumps movement function
/*****************************************/
function moverbombas()
  {
  for (var n=1;n<=2;n++)
    {
    if (pumpsm[n-1]==1)
      {
      var box = circulos[n].getBBox();

      var CenterX=box.x + box.width / 2;
      var CenterY=box.y + box.height / 2;

      angs[n-1]=angs[n-1]+10;
      if (angs[n-1]>=360) angs[n-1]=0;

      embolos[n].setAttribute("transform","rotate("+angs[n-1].toString()+" "+CenterX.toString()+" "+CenterY.toString()+")");
      }
    }
  }
 
/*****************************************/
// AQUA movement function
/*****************************************/ 
function mueveagua(llenado)
	{
	// GET FONDO ATRIBUTES
	var afheight=parseInt(aguafondo.getAttribute("height"));	
	var afy=parseInt(aguafondo.getAttribute("y"));
	
	// CALC AGUA ATRIBUTES
	var height=(afheight*llenado)/100;
	var y=afy+afheight-height;

	// SET AGUA ATRIBUTES
	agua.setAttribute("height",height);
	agua.setAttribute("y",y);
	}
	
/*****************************************/
// HSV TO RGB
// accepts parameters
// h  Object = {h:x, s:y, v:z}
// OR
// h, s, v
/*****************************************/ 
function HSVtoRGB(h, s, v) {
	var r, g, b, i, f, p, q, t;
	if (arguments.length === 1) {
		s = h.s, v = h.v, h = h.h;
	}
	i = Math.floor(h * 6);
	f = h * 6 - i;
	p = v * (1 - s);
	q = v * (1 - f * s);
	t = v * (1 - (1 - f) * s);
	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
	return {
		r: Math.round(r * 255),
		g: Math.round(g * 255),
		b: Math.round(b * 255)
	};
}