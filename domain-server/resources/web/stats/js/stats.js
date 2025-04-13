$(document).ready(function(){


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
          // this looks like a number - give it the clickable class
          $(this).addClass('graphable-stat');
        }
      });

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
