$(document).ready ->
    socket = io.connect window.location
    lastSeen = 0
    $('.toprow .state').hide()
    $('#stop').on 'click', ->
        console.log "stop"
        socket.emit "stop", 1

    updateConnection = ->
        if new Date().getTime() - lastSeen > 3000
            console.log "last seen: #{new Date().getTime() - lastSeen} ago"
            $('#connection').removeClass('connected')
        else
            $('#connection').addClass('connected')
        window.setTimeout(updateConnection, 1000)

    updateConnection()

    socket.on 'live', ->
        console.log 'live'

    $('#mover').on 'mouseup', (evt) ->
        console.log "mover at: #{evt.target.value}"
        socket.emit "move to", evt.target.value

    socket.on 'position', (data) ->
        console.log "position: #{data[0]}"
        $('#position')[0].value = data[0]

    socket.on 'state', (data) ->
        console.log "state: #{data}"
        $('.toprow .state').hide()
        state = {'-1': 'stopping', '0': 'rest', '1': 'moving'}[data[0]]
        console.log "state: #{state}"
        $("##{state}").show()

    socket.on 'alive', (data) ->
        console.log 'hearbeat'
        lastSeen = new Date().getTime()
        $('#position')[0].value = data[0]

    socket.on 'direction', (data) ->
        console.log "direction: #{data[0]}"
        $('#direction').text(data[0])
