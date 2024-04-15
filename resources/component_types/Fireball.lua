Fireball = {

	OnStart = function(self)
		-- Register this enemy in the global table.
		table.insert(all_fireballs, self)
	end
}

