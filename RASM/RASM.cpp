#include <iostream>
#include <boost/timer/timer.hpp>
#include "Parsing/ParseCMD.h"

int main(int argc, const char* argv[])
{
	//const char* argvg[]  = {
	//"-target", "RDR2", 
	//"-platform", "PSX", 
	//"-dec", "Test/fishing_core.osc", "Test/fishing_core.osa",
	//"-ow", "-v", "-dcn"};

	//const char* argvg[]  = {
	//"-target", "RDR2", 
	//"-platform", "PC", 
	//"-dec", "Test/shop_gunsmith.ysc.full", "Test/shop_gunsmith.ysc.full.ysa",
	//"-ow", "-v"};

	//const char* argvg[]  = {
	//"-target", "RDR2", 
	//"-platform", "PC", 
	//"-dec", "Test/fishing_core.ysc.full", "Test/fishing_core.ysc.full.ysa",
	//"-ow"};

	//const char* argvg[] = {
	//"-target", "GTAV",
	//"-platform", "XBOX",
	//"-dec", "Test/freemode.xsc", "Test/freemode.xsa",
	//"-ow", "-v"};

	//const char* argvg[] = {
	//"-target", "GTAV",
	//"-platform", "PSX",
	//"-dec", "Test/AntiModder.csc", "Test/AntiModder.csa",
	//"-ow" };

	//const char* argvg[] = {
	//"-target", "GTAIV",
	//"-platform", "XBOX",
	//"-dec", "Test/modslot10.sco", "Test/modslot10.sca",
	//"-ow", "-v"};

	//const char* argvg[] = {
	//"-target", "RDR",
	//"-platform", "XBOX",
	//"-dec", "Test/freemodeRDR.xsc", "Test/freemodeRDR.xsa",
	//"-ow" , "-v"};

	//const char* argvg[] = {
	//"-target", "GTAIV",
	//"-platform", "XBOX",
	//"-dec", "Test/e2_freemode.sco", "Test/e2_freemode.sca",
	//"-ow" , "-v"};

	//const char* argvg[] = {
	//"-target", "GTAV",
	//"-platform", "PC",
	//"-dec", "Test/agency_heist1.ysc.full", "Test/agency_heist1.ysc.full.ysa",
	//"-ow", "-v"};

	//const char* argvg[] = {
	//"-target", "RDR2",
	//"-platform", "PC",
	//"-com", "Test/shop_gunsmith.ysc.full.ysa", "Test/shop_gunsmith.ysc.full.out",
	//"-ow" };
	//
	//

	//const char* argvg[] = {
	//"-target", "GTAV",
	//"-platform", "PC",
	//"-com", "Test/agency_heist1.ysc.full.ysa", "Test/agency_heist1.ysc.full.compiled.ysc",
	//"-ow"};
	

	//const char* argvg[] = {
	//"-target", "GTAV",
	//"-platform", "XBOX",
	//"-com", "Test/freemode.xsa", "Test/freemode.compiled.xsc",
	//"-ow"};

	//const char* argvg[] = {
	//"-target", "RDR",
	//"-platform", "XBOX",
	//"-com", "Test/freemodeRDR.xsa", "Test/freemodeRDR.compiled.xsc",
	//"-ow"};

	const char* argvg[] = {
	"-target", "GTAIV",
	"-platform", "XBOX",
	"-com", "Test/e2_freemode.sca", "Test/e2_freemode.compiled.sco",
	"-ow"};
	
	argv = argvg;
	argc = 8;
repeat:
	boost::timer::cpu_timer Time;
	int ReturnCode = 0;
	
	if (argc > 0)
	{
		ReturnCode = ParseCommandLine(argc, argv);
	}
	else 
		return EXIT_FAILURE;
	
	std::cout << Time.format();
	Utils::System::Pause();

	//const char* argvg2[] = {
	//"-target", "RDR2",
	//"-platform", "PC",
	//"-dec", "Test/shop_gunsmith.ysc.full.out", "Test/shop_gunsmith.ysc.full.out.ysa",
	//"-ow", "-v", "-dcn" };


	//const char* argvg2[] = {
	//"-target", "GTAV",
	//"-platform", "PC",
	//"-dec", "Test/agency_heist1.ysc.full.compiled.ysc", "Test/agency_heist1.ysc.full.compiled.ysa",
	//"-ow", "-v", "-dcn" };

	//const char* argvg2[] = {
	//"-target", "GTAV",
	//"-platform", "XBOX",
	//"-dec", "Test/freemode.compiled.xsc", "Test/freemode.compiled.xsa",
	//"-ow", "-v", "-dcn" };
	//const char* argvg2[] = {
	//"-target", "RDR",
	//"-platform", "XBOX",
	//"-dec", "Test/freemodeRDR.compiled.xsc", "Test/freemodeRDR.compiled.xsa",
	//"-ow", "-v", "-dcn" };
	const char* argvg2[] = {
	"-target", "GTAIV",
	"-platform", "XBOX",
	"-dec", "Test/e2_freemode.compiled.sco", "Test/e2_freemode.compiled.sca",
	"-ow", "-v", "-dcn" };


	argv = argvg2;
	argc = 10;
	
	goto repeat;

	
	return ReturnCode;
}

