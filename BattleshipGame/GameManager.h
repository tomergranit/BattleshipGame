#pragma once

#include <memory>
#include "BattleBoard.h"
#include "IBattleshipGameAlgo.h"
#include "IGameVisual.h"

using std::shared_ptr;
using std::unique_ptr;

namespace battleship
{
	/** Special attack coordinates returned when the player chooses not to attack but to forfeit the game. */
	const int FOREFEIT_COORDINATES = -1;

	/** Manages a session of a single game and keeps the score of past games */
	class GameManager
	{
	public:
		GameManager();
		virtual ~GameManager();

		/** Starts a new game session using the given board, between the 2 players algorithms.
		 *  The game output will be depicted using the visualizer strategy.
		 */
		void startGame(shared_ptr<BattleBoard> board,
					   IBattleshipGameAlgo& playerA, IBattleshipGameAlgo& playerB,
					   IGameVisual& visualizer);

	private:
		// Score for past games
		int _playerAWins = 0;
		int _playerBWins = 0;

		/** Helper methods: Are all game pieces of player X gone? */
		bool isPlayerShipsLeft(BattleBoard* board, PlayerEnum player) const;

		/** Returns if both player forfeit or anybody have won */
		bool isGameOver(BattleBoard* board, bool isPlayerAForfeit, bool isPlayerBForfeit) const;

		/** Takes into consideration the current game state and points currPlayer to the next player who
		 *  should be playing
		 */
		IBattleshipGameAlgo* switchPlayerTurns(IBattleshipGameAlgo& playerA, IBattleshipGameAlgo& playerB,
											   IBattleshipGameAlgo* currPlayer,
											   bool isPlayerAForfeit, bool isPlayerBForfeit) const;

		/** Updates the game points when a game piece have been sank */
		void updateCurrentGamePoints(const GamePiece *const sankPiece, int& playerAScore, int& playerBScore) const;

		/** Update the number of wins for each player according to the final game state */
		void updateScoreboard(const BattleBoard *const board);
	};
}
