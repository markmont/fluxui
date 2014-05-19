
function fluxuiCtrl($scope) {

/* Testing: */
$scope.displayResult = function( value ) {
    console.log( "displayResult() called: " + value );
    $scope.testResult = value;
    $scope.$apply();
};

/* Testing: */
$scope.getResult = function() {
    console.log( "getResult() called" );
    runtest( $scope.displayResult );
};


/*
 * Connect to Flux
 * 
 */

/* Callback function */
$scope.connectToFluxDone = function( statusText ) {
    $scope.connectionStatus = statusText;
    $scope.$apply();
};

$scope.connectToFlux = function() {
    nacl_module.postMessage( makeCall( 'connectToFlux',
        $scope.connectToFluxDone ) );
};


/*
 * Disconnect from Flux
 * 
 */

/* Callback function */
$scope.disconnectFromFluxDone = function( statusText ) {
    $scope.connectionStatus = statusText;
    $scope.$apply();
};

$scope.disconnectFromFlux = function() {
    nacl_module.postMessage( makeCall( 'disconnectFromFlux',
        $scope.disconnectFromFluxDone ) );
};


}
