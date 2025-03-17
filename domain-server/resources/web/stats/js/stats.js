$(document).ready(function(){

  var currentHighchart;

  // setup a function to grab the nodeStats
  function getNodeStats() {

    var uuid = qs("uuid");

    $.getJSON("/nodes/" + uuid + ".json", function(json){

      // update the table header with the right node type
      $('#stats-lead h3').html(json.node_type + " stats (" + uuid + ")");

      delete json.node_type;

      var stats = JsonHuman.format(json);

      $('#stats-container').html(stats);

      // add the clickable class to anything that looks like a number
      $('.jh-value span').each(function(val){
        console.log(val);
        if (!isNaN($(this).text())) {
          // this looks like a number - give it the clickable class so we can get graphs for it
          $(this).addClass('graphable-stat');
        }
      });

      if (currentHighchart) {
        // get the current time to set with the point
        var x = (new Date()).getTime();

        // get the last value using underscore-keypath
        var y = Number(_(json).valueForKeyPath(graphKeypath));

        // start shifting the chart once we hit 20 data points
        var shift = currentHighchart.series[0].data.length > 20;
        currentHighchart.series[0].addPoint([x, y], true, shift);
      }
    }).fail(function(data) {
      $('#stats-container th').each(function(){
        $(this).addClass('stale');
      });
    });
  }

  // do the first GET on page load
  getNodeStats();
  // grab the new assignments JSON every second
  var getNodeStatsInterval = setInterval(getNodeStats, 1000);
});
