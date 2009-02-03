
/* This is an automatically generated file,
don't modify it by hand*/

#include <public/models.h>
#include <public/simulation.h>

/* antenna models include files */

#include <antenna/omni_antenna.h>

/* application models include files */

/* interference models include files */

#include <interference/no_interference.h>
#include <interference/ortho_interference.h>
#include <interference/cdma_interference.h>

/* mac models include files */

/* mobility models include files */

#include <mobility/static_static_mobility.h>
#include <mobility/random_static_mobility.h>
#include <mobility/random_billiard_mobility.h>
#include <mobility/static_billiard_mobility.h>

/* modulation models include files */

#include <modulation/dumb_modulation.h>
#include <modulation/bpsk_modulation.h>
#include <modulation/threshold_modulation.h>

/* propagation models include files */

#include <propagation/pathloss_propagation.h>
#include <propagation/no_fading_propagation.h>
#include <propagation/range_propagation.h>

/* radio models include files */


/* queue models include files */


/* battery models include files */


/* main function */

int main (int argc, char ** argv) {

	models_add_antenna     ( "OMNI",
	                         omni_antenna_instantiate,
	                         omni_antenna_compute_tx,
	                         omni_antenna_compute_rx,
	                         omni_antenna_get_ioctl,
	                         omni_antenna_set_ioctl,
	                         omni_antenna_complete);

	models_add_interference( "NONE",
	                         no_interference_instantiate,
	                         no_interference_correlation,
	                         no_interference_complete);

	models_add_interference( "ORTHO",
	                         ortho_interference_instantiate,
	                         ortho_interference_correlation,
	                         ortho_interference_complete);

	models_add_interference( "CDMA",
	                         cdma_interference_instantiate,
	                         cdma_interference_correlation,
	                         cdma_interference_complete);

	models_add_mobility    ( "STATIC_STATIC",
	                         static_static_mobility_instantiate,
	                         static_static_mobility_update,
	                         static_static_mobility_complete );

	models_add_mobility    ( "RANDOM_STATIC",
	                         random_static_mobility_instantiate,
	                         random_static_mobility_update,
	                         random_static_mobility_complete );

	models_add_mobility    ( "RANDOM_BILLIARD",
	                         random_billiard_mobility_instantiate,
	                         random_billiard_mobility_update,
	                         random_billiard_mobility_complete );

	models_add_mobility    ( "STATIC_BILLIARD",
	                         static_billiard_mobility_instantiate,
	                         static_billiard_mobility_update,
	                         static_billiard_mobility_complete );

	models_add_modulation  ( "DUMB",
	                         dumb_modulation_instantiate,
	                         dumb_modulation_compute_BER,
	                         dumb_modulation_complete );

	models_add_modulation  ( "BPSK",
	                         bpsk_modulation_instantiate,
	                         bpsk_modulation_compute_BER,
	                         bpsk_modulation_complete );

	models_add_modulation  ( "THRESHOLD",
	                         threshold_modulation_instantiate,
	                         threshold_modulation_compute_BER,
	                         threshold_modulation_complete );

	models_add_propagation    ( "PATHLOSS",
	                         pathloss_propagation_instantiate,
	                         pathloss_propagation_propagation,
	                         pathloss_propagation_complete );

	models_add_propagation    ( "NO_FADING",
	                         no_fading_propagation_instantiate,
	                         no_fading_propagation_propagation,
	                         no_fading_propagation_complete );

	models_add_propagation    ( "RANGE",
	                         range_propagation_instantiate,
	                         range_propagation_propagation,
	                         range_propagation_complete );

	/* Initialize simulation */

	if (simulation_init(argc, argv) < 0) {
		fprintf(stderr, "failed in simulation_init\n");
		return -1;
	}

	/* Start simulation */

	simulation_start();
	return 0;
}
