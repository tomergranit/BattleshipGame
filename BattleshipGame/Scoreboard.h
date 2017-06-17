#pragma once

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <functional>
#include "GameManager.h"

using std::shared_ptr;
using std::vector;
using std::unordered_map;
using std::map;
using std::string;
using std::mutex;
using std::condition_variable;
using std::function;

namespace battleship
{
	struct PlayerStatistics
	{
		const string playerName;
		const int pointsFor;
		const int pointsAgainst;
		const int wins;
		const int loses;
		const float rating;

		PlayerStatistics(const string& aPlayerName);
		PlayerStatistics(const string& aPlayerName, int aPointsFor, int aPointsAgainst, int aWins, int aLoses);

		PlayerStatistics updateStatistics(int addedPointsFor, int addedPointsAgainst,
										  bool isWin, bool isLose) const;
	};

	struct PlayerStatisticsRatingSort {
		bool operator()(const PlayerStatistics& a, const PlayerStatistics& b) const
		{
			return a.rating < b.rating;
		}
	};

	struct RoundResults
	{
		int roundNum;
		set<PlayerStatistics, PlayerStatisticsRatingSort> playerStatistics;

		RoundResults(int aRoundNum);
	};

	/**
	 *	Scoreboard for managing number of matches each player is enlisted in,
	 *  and the total number of points each player have accumulated so far.
	 *  This class does not validate, and assumes all player "strings" are valid players.
	 */
	class Scoreboard
	{
	public:
		Scoreboard(vector<string> players);
		virtual ~Scoreboard() = default;

		/** Registers the two players for a future match.
		 *  -- This method is not thread safe.
		 */
		void registerMatch(const string& playerA, const string& playerB);

		/** Update the score table with the game results.
		 *  This method is thread safe.
		 */
		void updateWithGameResults(shared_ptr<GameResults> results,
								   const string& playerAName, const string& playerBName);

		/**
		 *  Returns total matches the player currently participates in
		 *  --This method is thread safe.
		 */
		int getPlayerEnlistedMatches(const string& player) const;

		/** A "queue" of round results for rounds that are finished being played.
		 *  Outside consumers are expected to pop entries from this data structure after processing them.
		 */
		vector<shared_ptr<RoundResults>>& getRoundResults();

		/** Waits on round results queue until new data is ready,
		 *  when it arrives - trigger the print results table function (as a callback)
		 */
		void waitOnRoundResults();

	private:

		/** Minimal space allocated for player name in the table (visual parameter) */
		static constexpr int MIN_PLAYER_NAME_SIZE = 12;

		// Number of player entries that must be present for a round to count as finished
		int _playersPerRound;

		// A mutex lock to protect the score table during updates from multiple worker thread updates
		mutex _scoreLock;

		// A mutex lock to protect the roundResults table during access time
		mutex _roundResultsLock;

		// A predicate to notify listeners on the _roundResults queue that new data is ready
		condition_variable _roundResultsCV;

		// Current points & statistics for each player, contains the most up to date info about each player
		map<string, PlayerStatistics> _score;

		// Number of registered matches for each player, so far.
		// This data structure is populated when the tournament is constructed, to make sure
		// player's games are evenly spread.
		unordered_map<string, int> _registeredMatches;

		// Tracked matches data - 
		// key is round number
		// value is RoundResults (that accumulates data from finished games for each player for that round)
		unordered_map<int, shared_ptr<RoundResults>> _trackedMatches;

		// Contains results of finished rounds of games.
		// This data is ready for printing and is kept here until queried by the reporter thread.
		// Data is pushed to this vector only when the round is finished.
		// _roundResults is volatile to make sure it's handled before condition_variables are notified
		vector<shared_ptr<RoundResults>> _roundsResults;

		// Holds the longest player name encountered
		int _maxPlayerNameLength;

		/** Update the score table with the results for a single player from a single match
		 */
		void updatePlayerGameResults(PlayerEnum player, const string& playerName, GameResults* results);

		/** Pop and print all round results ready in the _roundResults queue
		 */
		void processRoundResultsQueue();

		/** Prints the round results in a formatted table to the console
		 */
		void printRoundResults(shared_ptr<RoundResults> roundResults) const;
	};
}
