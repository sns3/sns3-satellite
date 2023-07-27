<!DOCTYPE html>
<html lang="en">

<head><script type="text/javascript" src="/_static/js/bundle-playback.js?v=1WaXNDFE" charset="utf-8"></script>
<script type="text/javascript" src="/_static/js/wombat.js?v=txqj7nKC" charset="utf-8"></script>
<script type="text/javascript">
  __wm.init("http://web.archive.org/web");
  __wm.wombat("http://www.sns3.org/content/home.php","20220530234904","http://web.archive.org/","web","/_static/",
	      "1653954544");
</script>
<link rel="stylesheet" type="text/css" href="/_static/css/banner-styles.css?v=S1zqJCYt" />
<link rel="stylesheet" type="text/css" href="/_static/css/iconochive.css?v=qtvMKcIJ" />
<!-- End Wayback Rewrite JS Include -->


    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta name="author" content="">

    <title>SNS3</title>

    <!-- Bootstrap Core CSS -->
    <link href="/web/20220530234904cs_/http://www.sns3.org/css/bootstrap.min.css" rel="stylesheet">

    <!-- Custom CSS -->
    <link href="/web/20220530234904cs_/http://www.sns3.org/css/sns3.css" rel="stylesheet">
	<link href="/web/20220530234904cs_/http://www.sns3.org/css/scrolling-nav.css" rel="stylesheet">
	<link rel="icon" type="image/png" href="/web/20220530234904im_/http://www.sns3.org/img/sns3-logo.png"/>
    <!-- Custom Fonts -->
    <link href="/web/20220530234904cs_/http://www.sns3.org/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">

    <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
    <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
    <!--[if lt IE 9]>
        <script src="https://oss.maxcdn.com/libs/html5shiv/3.7.0/html5shiv.js"></script>
        <script src="https://oss.maxcdn.com/libs/respond.js/1.4.2/respond.min.js"></script>
    <![endif]-->

</head>
<script src="http://web.archive.org/web/20220530234904js_/https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js"></script>


<body>

    <!-- Navigation -->
    
 <nav class="navbar navbar-inverse navbar-fixed-top navbar-custom" role="navigation">
        <div class="container">
            <!-- Brand and toggle get grouped for better mobile display -->
            <div class="navbar-header">
				<div id="imgLogo">
					<img class="logo" src="/web/20220530234904im_/http://www.sns3.org/img/sns3-logo-white.png" style="max-width:60%" onclick="document.location.href = 'home.php'" alt=""></img>
				</div>
            </div>
            <!-- Collect the nav links, forms, and other content for toggling -->
            <div class="collapse navbar-collapse" id="bs-example-navbar-collapse-1">
                <ul class="nav navbar-nav navbar-right">
                    <li class="page-scroll">
                        <a href="http://web.archive.org/web/20220530234904/https://github.com/sns3/sns3-satellite"><p class="text-custom">Github</p></a>
                    </li>
                    <li class="page-scroll">
                        <a href="doc/satellite.html"><p class="text-custom">Documentation</p></a>
                    </li>
                    <li class="page-scroll">
                        <a href="api/index.html"><p class="text-custom">API</p></a>
                    </li>
                    <li class="page-scroll">
                        <a href="references.php"><p class="text-custom">References</p></a>
                    </li>
                    <li class="page-scroll">
                        <a href="commitee.php"><p class="text-custom">Steering commitee</p></a>
                    </li>
                </ul>
            </div>
            <!-- /.navbar-collapse -->
        </div>
        <!-- /.container -->
</nav>

    <!-- Header Carousel -->
    <header id="myCarousel" class="carousel slide">
	<section>
		<div class="container">
			<!-- Marketing Icons Section -->
			<div class="row">
				<div class="col-lg-12">
                    <h1 class="page-header">Welcome to SNS3</h1>
				</div>
			</div>
            <div class="row">
                <div class="col-md-6">
                    <center>
                        <p>Satellite Network Simulator 3 (SNS3) is a satellite network extension to Network Simulator 3 (ns-3) platform. 
                        ns-3 is a discrete event simulator for networking research licensed under General Public License v2 (GPLv2). 
                        SNS3 models a full interactive multi-spot beam satellite network with a geostationary satellite and transparent star 
                        (bent-pipe) payload. The reference satellite system consists of 72 spot-beams with an European coverage, 5 gateways 
                        and Ka-band frequencies. It implements the <abbr style="text-decoration:none" title="Digital video broadcasting - Return Channel via 
                        Satellite 2nd generation">DVB-RCS2</abbr> - <abbr style="text-decoration:none" title="Digital video Broadcasting - Second">DVB-S2</abbr> 
                        standards.</p>
                        <br/>
                        <p>This platform has been initially developed by <a href="http://web.archive.org/web/20220530234904/http://satellite-ns3.com/">Magister Solutions Ltd</a> in the frame of 
                            <a href="http://web.archive.org/web/20220530234904/https://esa.int/">ESA</a> ARTES european projects (AO6947, AO7476).</p>
                        <p>It is now also promoted by <a href="http://web.archive.org/web/20220530234904/https://cnes.fr/en">CNES</a> (French Space Agency) as a reference 
                            open-source software tool within its research and development studies and activities in the domain of 
                            satellite communication systems and networks.</p>
                    </center>
                </div>
                <div class="col-md-6">
                    <center>
                        <img src="/web/20220530234904im_/http://www.sns3.org/img/thd-sat.jpg" style="max-width:70%"/>
                    </center>
                </div>
			</div>
		</div>
	</section>
	<section class="font-blue">
	<div class="container">
		<div class="row">
			<div class="col-md-6">
				<h2>SNS3 overview</h2>
                <br/>
            </div>
        </div>
		<div class="row">
			<div class="col-md-6">
                <p>SNS3 implements a wide range of DVB-S2/RCS2 mechanisms:
                    <ul>
                        <li> Adaptive Coding and Modulation (ACM)
                        <li>Return and forward link scheduling
                        <li>Demand Assignment Multiple Access (DAMA): Constant Rate Allocation (CRA), Rate-Based Dynamic Capacity (RBDC), Volume-Based Dynamic Capacity (VBDC), Free Capacity Allocation (FCA)
                        <li>Random Access (RA): Slotted ALOHA and Contention Resolution Diversity Slotted ALOHA (CRDSA)
                        <li>Generic Stream Encapsulation (GSE)
                        <li>Return Link Encapsulation (RLE)
                        <li>Automatic Repeat reQuest (ARQ) through LLC bidirectionnal convergence layer
                        <li>Multi-beam satellite antenna gain patterns
                        <li>Packet-by-packet SNIR computation
                        <li>Packet-by-packet co-channel and intra-beam interference tracking
                        <li>Weather traces, Land-Mobile Satellite (LMS) channel
                        <li>Error models based on BER curves
                    </ul>
            </div>
            <div class="col-md-6">
				<center>
                    <img src="/web/20220530234904im_/http://www.sns3.org/img/sns3-spot-beam.jpg" style="max-height:100%; max-width:100%"/>
				</center>
            </div>
		</div>
	</div>
	</section>
	<section>
	<div class="container">
		<div class="row">
			<div class="col-md-12">
				<h2>SNS3 Architecture</h2>
				<br/>
			</div>
		</div>
		<div class="row">
			<div class="col-md-12">
                <div>
                    <p>Built on the general-purpose NetDevice model of ns3, SNS3 proposes a realistic scenario corresponding to a High-Throughtput-Satellite (HTS) system,
                        consisting in 72 spotbeams and 5 gateways.
                    </p>
                </div>
                <br/>
                <div>
                    <center>
                        <img src="/web/20220530234904im_/http://www.sns3.org/img/features-sns3.png" style="max-height:80%; max-width:80%"/>
                    </center>
				</div>
			</div>
			
		</div>
	</div>
	</section>
	<section class="font-blue">
	<div class="container">
        <div class="row">
            <div class="col-lg-12">
                <h2>Contributors</h2>
            </div>
            <br/>
            <br/>
        </div>
        <div class="row">
            <div class="col-md-4">
                <center>
                    <img class="img_hover" src="/web/20220530234904im_/http://www.sns3.org/img/magister-footer-logo2.png" style="max-width:130%" onclick="document.location.href ='http://web.archive.org/web/20220530234904/http://www.magister.fi/'"/>
                </center>
            </div>
            <div class="col-md-4">
                <center>
                    <img class="img_hover" src="/web/20220530234904im_/http://www.sns3.org/img/ESA_logo2.png" style="max-width:130%" onclick="document.location.href = 'http://web.archive.org/web/20220530234904/https://esa.int'"/>
                </center>
            </div>
            <div class="col-md-4">
                <center>
                    <img class="img_hover" src="/web/20220530234904im_/http://www.sns3.org/img/cnes_logo_small2.png" style="max-width:130%" onclick="document.location.href = 'http://web.archive.org/web/20220530234904/https://cnes.fr/en'"/>
                </center>
            </div>
        </div>
    </div>
    </section>
    <div>
        <!-- Footer -->
		<footer>
	<div class="container">
            <div class="row">
                <div class="col-lg-12">
					<div id="licenses">
						<p style="font-size:90%">Contact <a href="http://web.archive.org/web/20220530234904/mailto:admin _AT_ sns3 _DOT_ org" title="SNS3 contact">
						   SNS3 Maintainers <i class="fa fa-envelope" aria-hidden="true"></i></a></p>
						<p style="font-size:90%">The text content of this website is published under the <a href="http://web.archive.org/web/20220530234904/http://creativecommons.org/licenses/by-nc-sa/3.0/" title="See license details" rel="license" hreflang="en">Creative Commons BY-NC-SA license</a> (except where otherwise
						   stated). Some sections icons are derivative work of the GPL2 icons provided by the Gnome
						   project.</p>
						<p style="font-size:90%">Logos and trademarks are the property of their respective owners. Any representation,
						   reproduction and/or exploitation, whether partial or total, of trademarks and logos
						   is prohibited without prior written permission from the owners.</p>
						<p style="font-size:90%"><a href="./privacy_policy.php" title="Read the privacy policy">
						   Privacy policy</a>.</p>
					</div>
                </div>
            </div>
	</div>
</footer>
    </div>
   

</body>
 <!-- /.container -->

    <!-- jQuery -->
    <script src="/web/20220530234904js_/http://www.sns3.org/js/jquery.js"></script>

    <!-- Bootstrap Core JavaScript -->
    <script src="/web/20220530234904js_/http://www.sns3.org/js/bootstrap.min.js"></script>

    <!-- Script to Activate the Carousel -->
    <script>
    $('.carousel').carousel({
        interval: 5000 //changes the speed
    })
    </script>
	
	<!-- Scrolling Nav JavaScript -->
    <script src="/web/20220530234904js_/http://www.sns3.org/js/jquery.easing.min.js"></script>
    <script src="/web/20220530234904js_/http://www.sns3.org/js/scrolling-nav.js"></script>

</html>
<!--
     FILE ARCHIVED ON 23:49:04 May 30, 2022 AND RETRIEVED FROM THE
     INTERNET ARCHIVE ON 17:07:45 Jul 26, 2023.
     JAVASCRIPT APPENDED BY WAYBACK MACHINE, COPYRIGHT INTERNET ARCHIVE.

     ALL OTHER CONTENT MAY ALSO BE PROTECTED BY COPYRIGHT (17 U.S.C.
     SECTION 108(a)(3)).
-->
<!--
playback timings (ms):
  captures_list: 132.883
  exclusion.robots: 0.26
  exclusion.robots.policy: 0.239
  RedisCDXSource: 0.874
  esindex: 0.015
  LoadShardBlock: 106.044 (3)
  PetaboxLoader3.datanode: 61.8 (4)
  load_resource: 137.601
  PetaboxLoader3.resolve: 105.797
-->