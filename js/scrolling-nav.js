var _____WB$wombat$assign$function_____ = function(name) {return (self._wb_wombat && self._wb_wombat.local_init && self._wb_wombat.local_init(name)) || self[name]; };
if (!self.__WB_pmw) { self.__WB_pmw = function(obj) { this.__WB_source = obj; return this; } }
{
  let window = _____WB$wombat$assign$function_____("window");
  let self = _____WB$wombat$assign$function_____("self");
  let document = _____WB$wombat$assign$function_____("document");
  let location = _____WB$wombat$assign$function_____("location");
  let top = _____WB$wombat$assign$function_____("top");
  let parent = _____WB$wombat$assign$function_____("parent");
  let frames = _____WB$wombat$assign$function_____("frames");
  let opener = _____WB$wombat$assign$function_____("opener");

//jQuery to collapse the navbar on scroll
$(window).scroll(function() {
    if ($(".navbar").offset().top > 50) {
        $(".navbar-fixed-top").addClass("top-nav-collapse");
		$(".logo").addClass("logo-collapse");
		$(".dropdown-menu").addClass("dropdown-collapse");
	
		//document.getElementById("logo").animate({height:'45px', width: '81px'});
    } else {
        $(".navbar-fixed-top").removeClass("top-nav-collapse");
		$(".logo").removeClass("logo-collapse");
		$(".dropdown-menu").removeClass("dropdown-collapse");
	
		//document.getElementById("logo").animate({height:'65px', width: '117px'});
    }
});




}
/*
     FILE ARCHIVED ON 16:54:51 Aug 04, 2022 AND RETRIEVED FROM THE
     INTERNET ARCHIVE ON 17:12:51 Jul 26, 2023.
     JAVASCRIPT APPENDED BY WAYBACK MACHINE, COPYRIGHT INTERNET ARCHIVE.

     ALL OTHER CONTENT MAY ALSO BE PROTECTED BY COPYRIGHT (17 U.S.C.
     SECTION 108(a)(3)).
*/
/*
playback timings (ms):
  captures_list: 60.098
  exclusion.robots: 0.128
  exclusion.robots.policy: 0.118
  cdx.remote: 0.066
  esindex: 0.01
  LoadShardBlock: 30.568 (3)
  PetaboxLoader3.datanode: 136.445 (5)
  load_resource: 1033.542
  PetaboxLoader3.resolve: 892.094
  loaddict: 116.184
*/